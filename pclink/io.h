#pragma once

#include <stdint.h>
#include <stdbool.h>

enum {
	SILVERLINK_VID = 0x0451, 
	SILVERLINK_PID = 0xE001,
};

struct packet
{
	uint8_t machine;
	uint8_t command;
	uint16_t length;
	void * data;
	uint16_t checksum;
};

struct varheader
{
	uint16_t size;
	uint8_t type;
	uint8_t name[8];
	uint8_t version;
	uint8_t type2;
} __attribute__((packed));

bool io_open_serial(char const * portName, int baud);

bool io_open_usb(int vid, int pid);

void io_close();

uint16_t calculateCheckSum(void const * data, uint16_t length);

void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length);

bool receivePacket(struct packet * packet);

/**
 * Sends an ACK with machine id 0x73
 */
void sendACK();

/**
 * Receives an ACK. If no ACK is received, the program
 * will be quit.
 */
void receiveACK();