#pragma once

#include <stdint.h>
#include <stdbool.h>

extern int serialPort;

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

uint16_t calculateCheckSum(void const * data, uint16_t length);

void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length);

bool receivePacket(struct packet * packet);