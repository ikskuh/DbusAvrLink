#include "debug.h"
#include <string.h>

#include "tokens.h"

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
	static char name[128];
	detokenize(name, header->name, 8);
	printf("%s", name);
	// dumpSafeString(header->name, 8);
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