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

static void assertAck()
{
	struct packet packet;
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.machine != 0x73 || packet.command != 0x56) {
		error_message("No ACK\n");
		return;
	}
}

static void ack()
{
	sendPacket(0x73, 0x56, NULL, 0);
}

static void list()
{
	struct packet packet;
	struct varheader header;
	
	memset(&header, 0, sizeof header);
	header.type = 0x19; // Directory
	
	sendPacket(0x73, 0xA2, &header, sizeof header);
	
	assertAck();
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to get size packet\n");
		return;
	}
	if(packet.command != 0x15) {
		error_message("Packet size is not data.\n");
		return;
	}
	uint16_t freeRAM = *((uint16_t*)packet.data);
	
	printf("free: %d\n", freeRAM);
	
	ack();
	
	do
	{
		if(receivePacket(&packet) == false) {
			error_message("Failed to receive listing packet.\n");
			return;
		}
	
		if(packet.command == 0x06)
		{
			struct varheader * h = packet.data;
			char name[128];
			detokenize(name, h->name, 8);
			
			if(h->type2 & 0x80) {
				printf("a");
			} else {
				printf("-");
			}
			char const * type = "<unknown>";
			switch(h->type)
			{
				case 0x00: type = "real"; break;
				case 0x01: type = "list"; break;
				case 0x02: type = "mat"; break;
				case 0x03: type = "y-var"; break;
				case 0x04: type = "str"; break;
				case 0x05: type = "pgm"; break;
				case 0x06: type = "pgm-l"; break;
				case 0x07: type = "pic"; break;
				case 0x08: type = "gdb"; break;
				case 0x0C: type = "cplx"; break;
				case 0x0D: type = "clist"; break;
				case 0x15: type = "avar"; break;
				case 0x17: type = "grp"; break;
			}
			
			printf("\t%s\t%s\n", type, name);
		}
	
		ack();
	} while(packet.command != 0x92); // while not EOT
	
}

int main(int argc, char ** argv)
{
	char const * portName = "/dev/ttyUSB1";
	
	serialPort = ttyOpen (portName, B9600);
	if(serialPort < 0) {
		error_message("Failed to open serial port %s\n", portName);
		return EXIT_FAILURE;
	}
	
	list();
	
	close(serialPort);
	
	return EXIT_SUCCESS;
}