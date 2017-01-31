#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>

#define error_message(...) fprintf(stderr, __VA_ARGS__)

int set_interface_attribs (int fd, int speed, int parity);

void set_blocking (int fd, int should_block);

int serialPort;

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

static void readabs(int fd, void * buffer, size_t nBytes);

bool receivePacket(struct packet * packet);

void extractVarHdr(struct packet const * packet, struct varheader * header);

void dumpHeader(struct varheader const * header);

void dumpPacket(struct packet const * packet);


void dumpSafeString(char const * str, uint16_t len);

int main(int argc, char ** argv)
{
	char const * portName = "/dev/ttyUSB1";
	
	serialPort = open (portName, O_RDWR | O_NOCTTY | O_SYNC);
	if (serialPort < 0)
	{
		error_message ("error %d opening %s: %s\n", errno, portName, strerror (errno));
		return 1;
	}

	set_interface_attribs (serialPort, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (serialPort, 1);
	
	bool success;
	
	struct packet packet;
	
	int state = 0;
	
	// Query if we have a calc at the other side... :)
	// sendPacket(0x83, 0x68, NULL, 0);
	
	while(true)
	{
		printf("wait...");
		success = receivePacket(&packet);
		
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
		
		if(success) {
			printf("Success :)\n");
			
			switch(packet.command)
			{
				case 0x68: // CHECK RDY
					// Let's say we are an TI83+ :)
					sendPacket(0x73, 0x56, NULL, 0);
					break;
				case 0x06: // Var Header
				{
					printf("Got VAR HDR, now ACK, CTS\n");
					// Accept the variable header by saying:
					// ACK, CTS
					sendPacket(0x73, 0x56, NULL, 0);
					sendPacket(0x73, 0x09, NULL, 0);
					break;
				}
				case 0x09: // CTS
				{
					if (state == 2) {
						// yay, now send some data
						printf("Send data!\n");
						char const * msg = "ABCDEF0123456789";
						
						sendPacket(
							0x73,
							0x15,
							msg,
							16);
					} else {
						error_message("CTS in invalid state!\n");
					}
					break;
				}
				case 0x92: // End Of transmittion
				{
					// ACK this shit
					sendPacket(0x73, 0x56, NULL, 0);
					break; 
				}
				case 0x15: // DATA
				{
					printf("File Contents:\n");
					printf("'");
					dumpSafeString(packet.data, packet.length);
					printf("'\n");
				
					// ACK Packet
					sendPacket(0x73, 0x56, NULL, 0);
					break;
				}
				case 0x56: // ACK
					// Answer ACK with CTS
					break;
					switch(state++)
					{
						case 0: // We got a "RDY" calc
						{
							printf("Send header!\n");
							
							struct varheader hdr = {
								0x0010,
								0x05,
								"CODE\0\0\0\0",
								0x00,
								0x00
							};
							
							sendPacket(0x73, 0x06, &hdr, sizeof hdr);
							
							break;
						}
						case 1:
						{
							// We got an ACK for the receiption of our header.
							// just be happy
							break;
						}
						default:
							error_message("Unsupported state: %d\n", state - 1);
							break;
					}
					break;
			}
			
		} else {
			printf("Failed :(\n");
		}
		if(packet.data) free(packet.data);
	}
	
	close(serialPort);
}


int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		error_message ("error %d from tcgetattr\n", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
														// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
														// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		error_message ("error %d from tcsetattr\n", errno);
		return -1;
	}
	return 0;
}

void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		error_message ("error %d from tggetattr\n", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		error_message ("error %d setting term attributes\n", errno);
}


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
		int len = read(fd, (void*)buffer, nBytes - i);
		if(len == -1) {
			error_message("Failed to read stuff from fd=%d\n", fd);
			return;
		}
		buffer += len;
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

void extractVarHdr(
	struct packet const * packet,
	struct varheader * header)
{
	memset(header, 0, sizeof *header);
	memcpy(header, packet->data, packet->length);
}

void dumpHeader(struct varheader const * header)
{
	printf("SIZ = %04X\n", header->size);
	printf("TYP = %02X\n", header->type);
	printf("NAM =");
	for(int i = 0; i < 8; i++) {
		printf(" %02X", header->name[i]);
	}
	
	printf(" \"");
	dumpSafeString(header->name, 8);
	printf("\"\n");
	printf("VER = %02X\n", header->version);
	printf("TP2 = %02X\n", header->type2);
}

void dumpPacket(struct packet const * packet)
{
	printf("MID = %02X\n", packet->machine);
	printf("CMD = %02X\n", packet->command);
	printf("LEN = %04X\n", packet->length);
	if(packet->data != NULL) {
		printf("DAT =");
		uint8_t * const data = packet->data;
		for(int i = 0; i < packet->length; i++) {
			printf(" %02X", data[i]);
		}
		printf("\n");
	} else {
		printf("DAT = NULL\n");
	}
	printf("CHK = %04X\n", packet->checksum);
}

void dumpSafeString(char const * str, uint16_t len)
{
	for(int i = 0; i < len; i++) {
		char c = str[i];
		if(c >= 0x20 && c < 127) {
			printf("%c", c);
		} else {
			printf(".");
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