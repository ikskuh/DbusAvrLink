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

// #define VERBOSE

/**
 * TODO:
 * - Implement "tokenizer" mode that will create a calculator string
 * - Add getopt argument setup.
 */

int main(int argc, char ** argv)
{
	bool silent = true;
	char const * portName = "/dev/ttyUSB1";
	char const * outputFile = "received.8xg"; // By default, receive into a group file.
	
	serialPort = ttyOpen(portName, B9600);
	if(serialPort < 0) {
		error_message("Failed to open serial port %s\n", portName);
		exit(EXIT_FAILURE);
	}
	
	bool success;
	struct packet packet;
	
	LOG("Waiting for files...\n");
	
	int count = 0;
	struct ti83f_entry * results = NULL;
	
	struct varheader latestHeader;
	
	if(silent)
	{
		latestHeader.size = 0;
		latestHeader.version = 0;
		latestHeader.type2 = 0;
		latestHeader.type = 0x05; // Program
		memset(latestHeader.name, 0, 8);
		memcpy(latestHeader.name, "DEMO", 4);
		sendPacket(0x03, 0xA2, &latestHeader, sizeof latestHeader);

		receiveACK();
	}
	
	bool receiving = true;
	while(receiving)
	{
		success = receivePacket(&packet);
		
#ifdef VERBOSE
		LOG("Packet:\n");
		dumpPacket(&packet);
		switch(packet.command) {
			case 0x06:
			case 0x88:
			case 0xA2:
			case 0xC9:
				LOG("Var Header:\n");
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
				LOG("Got VAR HDR, now ACK, CTS\n");
				
				memcpy(&latestHeader, packet.data, sizeof latestHeader);
				
				char name[128];
				detokenize(name, latestHeader.name, 8);
				
				LOG("Received VAR header: %s\n", name);
				
				// Accept the variable header by saying:
				// ACK, CTS
				sendPacket(0x73, 0x56, NULL, 0);
				sendPacket(0x73, 0x09, NULL, 0);
				break;
			}
			case 0x92: // End Of transmittion
			{
				// ACK this shit
				sendACK();
				receiving = false;
				break; 
			}
			case 0x36: // EXIT
			{
				receiving = false;
				sendACK();
				break;
			}
			case 0x15: // DATA
			{
				LOG("Receive data...\n");
				results = realloc(results, (++count) * sizeof(struct ti83f_entry));
				
				results[count - 1].type  = latestHeader.type;
				results[count - 1].size  = packet.length;
				results[count - 1].data  = packet.data;
				results[count - 1].flags = (latestHeader.type2 & 0x80) ? TI83F_ARCHIVED : 0;
				memcpy(results[count - 1].name, latestHeader.name, 8);
			
				packet.data = NULL; // prevent deletion
				packet.length = 0;
				
				// ACK Packet
				sendPacket(0x73, 0x56, NULL, 0);
				
				if(silent) receiving = false;
				
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
	
	if(count > 0)
	{
		LOG("Transmission finished, now storing data...\n");
		
		struct ti83f_file file;
		
		sprintf(file.comment, "File created by %s.", argv[0]);
		file.length = count;
		file.entries = results;
		
		FILE * f = fopen(outputFile, "wb");
		ti83f_store(f, &file);
		fclose(f);
	}
	else
	{
		LOG("No files were transmitted, no output file generated.\n");
	}
	
	LOG("Done.\n");
	close(serialPort);
}

