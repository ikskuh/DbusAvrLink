#include "io.h"
#include "debug.h"
#include <stdlib.h>
#include <unistd.h>

int serialPort = -1;

void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length)
{
	write(serialPort, &machineId, 1);
	write(serialPort, &command, 1);
	write(serialPort, &length, 2);
	if(data != NULL) {
		write(serialPort, data, length);
		uint16_t cs = calculateCheckSum(data, length);
		write(serialPort, &cs, 2);
	}
}

static void readabs(int fd, void * buffer, size_t nBytes)
{
	uintptr_t ptr = (uintptr_t)buffer;
	for(size_t i = 0; i < nBytes;)
	{
		int len = read(fd, (void*)ptr, nBytes - i);
		if(len == -1) {
			error_message("Failed to read stuff from fd=%d\n", fd);
			return;
		}
		ptr += len;
		i += len;
	}
}

bool receivePacket(struct packet * packet)
{
	readabs(serialPort, &packet->machine, 1);
	readabs(serialPort, &packet->command, 1);
	readabs(serialPort, &packet->length, 2);
	switch(packet->command) {
		case 0x06:
		case 0x15:
		case 0x36:
		case 0x88:
		case 0xA2:
		case 0xC9:
		{
			packet->data = malloc(packet->length);
			readabs(serialPort, packet->data, packet->length);
			uint16_t cs0 = calculateCheckSum(packet->data, packet->length);
			readabs(serialPort, &packet->checksum, 2);
			
			if(cs0 != packet->checksum) {
				error_message("Invalid Checksum: %04X ≠ %04X\n", packet->checksum, cs0);
			}
			
			return (cs0 == packet->checksum);
		}
		default:
		{
			packet->length = 0;
			packet->data = 0;
			packet->checksum = 0xFFFF;
			return true;
		}
	}
}


uint16_t calculateCheckSum(void const * data, uint16_t length)
{
	uint16_t checksum = 0;
	uint8_t const * _data  = data;
	for(int i = 0; i < length; i++) {
		checksum += _data[i];
	}
	return checksum;
}
