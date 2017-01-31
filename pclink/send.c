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

void transmit()
{
	struct packet packet;
	
	struct{
		uint16_t size;
		uint8_t msg[16];
	} msg = {
		0x0010,
		"0123456789ABCDEF",
	};
	
	struct varheader header =
	{
		msg.size + 2,
		0x05, // Program
		{ 'A', 'B', 'C', 0x00 },
		0x00,
		0x00,
	};
	
	error_message("Trying to send Str1\n");
	
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
	
	sendPacket(0x73, 0x06, &header, sizeof header);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x56) {
		error_message("No VAR-ACK\n");
		return;
	}
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x09) {
		error_message("No VAR-CTS\n");
		return;
	}
	// ACK-CTS
	sendPacket(0x73, 0x56, NULL, 0);
	
	// Send Data here
	sendPacket(0x73, 0x15, &msg, sizeof msg);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x56) {
		error_message("No DATA-ACK\n");
		return;
	}
	
	
	// End of Transmittion
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

void debug()
{
	struct packet packet;
	int i = 0;
	while(true)
	{
		if(receivePacket(&packet) == false) {
			error_message("Invalid packet :(\n");
			continue;
		}
		
		printf("Packet %4d:\n", ++i);
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
	}
}

void transmitSilent()
{
	struct packet packet;
	
	struct{
		uint16_t size;
		uint8_t msg[16];
	} msg = {
		0x0010,
		"0123456789ABCDEF",
	};
	
	struct varheader header =
	{
		msg.size + 2,
		0x05, // Program
		{ 'A', 'B', 'C', 0x00 },
		0x00,
		0x00,
	};
	
	error_message("Trying to send prgmABC silently \n");
	
	// Query if we have a calc at the other side... :)
	sendPacket(0x23, 0x68, NULL, 0);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.machine != 0x73 || packet.command != 0x56) {
		error_message("No RDY-ACK\n");
		return;
	}
	
	sendPacket(0x23, 0xC9, &header, sizeof header);
	
	debug();
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x56) {
		error_message("No VAR-ACK\n");
		return;
	}
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x09) {
		error_message("No VAR-CTS\n");
		return;
	}
	// ACK-CTS
	sendPacket(0x23, 0x56, NULL, 0);
	
	// Send Data here
	sendPacket(0x23, 0x15, &msg, sizeof msg);
	
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		return;
	}
	if(packet.command != 0x56) {
		error_message("No DATA-ACK\n");
		return;
	}
	
	
	// End of Transmittion
	sendPacket(0x23, 0x92, NULL, 0);
	
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
	
	serialPort = ttyOpen (portName, B9600);
	
	// transmit();
	// transmitSilent();
	
	debug();
	
	close(serialPort);
}