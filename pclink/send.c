#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <signal.h>

#include "tokens.h"
#include "debug.h"
#include "io.h"
#include "tty.h"
#include "ti83f.h"

void transmit(struct ti83f_file * file)
{
	struct packet packet;
	
	LOG("Query calculator.\n");
	
	// Query if we have a calc at the other side... :)
	sendPacket(0x83, 0x68, NULL, 0);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.machine != 0x73 || packet.command != 0x56) {
		error_message("No RDY-ACK\n");
		return;
	}
	
	for(uint32_t i = 0; i < file->length; i++)
	{
		struct ti83f_entry * entry = &file->entries[i];
		
		struct varheader header =
		{
			entry->size,
			entry->type,
			{ 0x00 },
			0x00,
			entry->flags,
		};
		memcpy(header.name, entry->name, 8);
		
		char name[128];
		detokenize(name, header.name, 8);
		
		LOG("Transmit file(%02X) %s\n", header.type, name);
		
		sendPacket(0x73, 0x06, &header, sizeof header);
		
		if(receivePacket(&packet) == false) {
			error_message("Failed to receive packet.\n");
			return;
		}
		if(packet.command != 0x56) {
			error_message("No VAR-ACK\n");
			return;
		}
		
		LOG("Wait for CTS...\n");
		
		int mode = 0;
		do
		{
			if(receivePacket(&packet) == false) {
				error_message("Failed to receive packet.\n");
				return;
			}
			LOG("Got packet(%02X)\n", packet.command);
			if(packet.command == 0x09) {
				mode = 1; // transmit
				break;
			}
			
			if(packet.command == 0x36) {
				// SKIP/EXIT
				uint8_t code = *((uint8_t*)packet.data);
				LOG("Rejection Code: %02X\n",code);
				if(code == 0x02) {
					mode = 2; // SKIP
					break;
				}
				if(code == 0x01) {
					mode = 3; // QUIT
					break;
				}
			}
			
			error_message("No VAR-CTS\n");
		} while(mode == 0);
		
		// ACK-CTS
		sendPacket(0x73, 0x56, NULL, 0);
		
		if(mode == 1) // SEND
		{
			LOG("Send data...\n");
			
			// Send Data here
			sendPacket(0x73, 0x15, entry->data, entry->size);
			
			if(receivePacket(&packet) == false) {
				error_message("Failed to receive packet.\n");
				return;
			}
			if(packet.command != 0x56) {
				error_message("No DATA-ACK\n");
				return;
			}
		}
		else if (mode == 3) // QUIT
		{
			break;
		}
	}
	
	LOG("End of Transmission...\n");
	
	// End of Transmission
	sendPacket(0x73, 0x92, NULL, 0);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x56) {
		error_message("No EOT-ACK\n");
		return;
	}
	
	error_message("Success!\n");
}

int main(int argc, char ** argv)
{
	char const * portName = "/dev/ttyUSB1";
	char const * sendFile = "testgroup.8xg";
	
	struct ti83f_file * file;
	
	FILE * f = fopen(sendFile, "rb");
	file = ti83f_load(f);
	fclose(f);
	
	if(file == NULL) {
		error_message("Could not read %s\n", sendFile);
		return EXIT_FAILURE;
	}
	
	serialPort = ttyOpen (portName, B9600);
	if(serialPort < 0) {
		error_message("Failed to open serial port %s\n", portName);
		return EXIT_FAILURE;
	}
	
	transmit(file);
	
	close(serialPort);
	
	return EXIT_SUCCESS;
}