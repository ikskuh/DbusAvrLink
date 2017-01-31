#include "ti83f.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"

static const uint8_t signature[] = 
{
	0x2A, 0x2A, 0x54, 0x49, 0x38, 0x33, 0x46, 0x2A, 0x1A, 0x0A, 0x00
};

int main(int argc, char ** argv)
{
	FILE * f = fopen("testgroup.8xg", "rb");
	
	struct ti83f header;
	
	fread(&header, 1, sizeof header, f);
	
	if(memcmp(header.sig, signature, sizeof signature) != 0) {
		error_message("Invalid file signature!\n");
		return 1;
	}
	
	printf("Comment:    %.*s\n", 42, header.comment);
	printf("Total Size: %u\n", (uint32_t)header.length);
	
	int idx = 0;
	for(int counter = 0; counter < header.length; )
	{
		struct ti83f_entry entry;
		counter += fread(&entry, 1, sizeof entry, f);
		
		if(entry.header != 0x0B && entry.header != 0x0D) {
			error_message("Invalid entry header: %d\n", (int)entry.header);
		}
		if(entry.length != entry.length2) {
			error_message("Size mismatch: %d ≠ %d\n", (int)entry.length, (int)entry.length2);
		}
		
		static char name[128];
		detokenize(name, entry.name, 8);
		
		printf("ENTRY %d\n", idx++);
		printf("\tLength:  %d\n",   (int)entry.length);
		printf("\tType:    %02X\n", (int)entry.type);
		printf("\tName:    %s\n", name);
		printf("\tVersion: %d\n", (int)entry.version);
		printf("\tFlags:   %d\n", (int)entry.flags);
		printf("\tLength': %d\n", (int)entry.length2);
		
		void * buffer = malloc(entry.length);
		
		counter += fread(buffer, 1, entry.length, f);
		
		free(buffer);
	}
	
	uint16_t checksumFile;
	fread(&checksumFile, 1, sizeof checksumFile, f);
	
	printf("Checksum: %04X\n", (int)checksumFile);
	
	LOG("Done.\n");
	
	fclose(f);
}