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

#define VERBOSE

int main(int argc, char ** argv)
{
	char const * portName = "/dev/ttyUSB1";
	
	serialPort = ttyOpen(portName, B9600);
	if(serialPort < 0) {
		error_message("Failed to open serial port %s\n", portName);
		exit(EXIT_FAILURE);
	}
	
	bool success;
	struct packet packet;
	
	printf("Waiting...\n");
	
	while(true)
	{
		success = receivePacket(&packet);
		
#ifdef VERBOSE
		printf("Packet:\n");
		dumpPacket(&packet);
		switch(packet.command) {
			case 0x06:
			case 0x88:
			case 0xA2:
			case 0xC9:
				printf("Var Header:\n");
				dumpHeader(packet.data);
				break;
		}
#endif
		
		if(!success) {
			error_message("Received packet with invalid checksum!\n");
			break; // stop on invalid data
		}
			
		switch(packet.command)
		{
			case 0x68: // CHECK RDY
				// Let's say we are an TI83+ :)
				sendPacket(0x73, 0x56, NULL, 0);
				break;
			case 0x06: // Var Header
			{
				printf("Got VAR HDR, now ACK, CTS\n");
				
				dumpHeader(packet.data);
				
				// Accept the variable header by saying:
				// ACK, CTS
				sendPacket(0x73, 0x56, NULL, 0);
				sendPacket(0x73, 0x09, NULL, 0);
				break;
			}
			case 0x92: // End Of transmittion
			{
				// ACK this shit
				sendPacket(0x73, 0x56, NULL, 0);
				printf("Done.\n");
				break; 
			}
			case 0x15: // DATA
			{
				printf("File Contents:\n");
				
				uint8_t * const data = packet.data;
				for(int i = 0; i < packet.length; i++) {
					printf(" %02X", data[i]);
				}
				printf("\n");
				
				printf("'");
				dumpSafeString(packet.data, packet.length);
				printf("'\n");
				
				static char msg[2048];
				
				detokenize(msg, (uint8_t*)packet.data + 2, packet.length - 2);
				
				printf("'");
				printf("%s", msg);
				printf("'\n");
			
				// ACK Packet
				sendPacket(0x73, 0x56, NULL, 0);
				break;
			}
			case 0x56: // ACK
			{
				// we are happy with simple acknowledge
				break;
			}
			default:
				error_message("Invalid command received: %02X", packet.command);
				break;
		}
		
		if(packet.data) free(packet.data);
	}
	
	close(serialPort);
}

