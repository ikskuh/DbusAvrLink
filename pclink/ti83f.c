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

struct raw_ti83f
{
	uint8_t sig[11];
	char comment[42];
	uint16_t length;
} __attribute__((packed));

struct raw_ti83f_entry
{
	uint16_t header;
	uint16_t length;
	uint8_t type;
	uint8_t name[8];
	uint8_t version;
	uint8_t flags;
	uint16_t length2;
}  __attribute__((packed));

int main(int argc, char ** argv)
{
	FILE * f = fopen("testgroup.8xg", "rb");
	
	struct ti83f_file * file = ti83f_load(f);
	if(file != NULL) {
		LOG("Success!\n");
	} else {
		LOG("Failure!\n");
	}
	ti83f_release(file);
	
	fclose(f);
}

struct ti83f_file * ti83f_load(FILE * stream)
{
	struct raw_ti83f header;
	fread(&header, 1, sizeof header, stream);
	if(memcmp(header.sig, signature, sizeof signature) != 0) {
		error_message("Invalid file signature!\n");
		return NULL;
	}
	
	struct ti83f_file * file = malloc(sizeof(struct ti83f_file));
	memset(file->comment, 0, sizeof file->comment);
	memcpy(file->comment, header.comment, sizeof header.comment);
	
	file->length = 0;
	file->entries = NULL;
	
	for(int counter = 0; counter < header.length; )
	{
		struct raw_ti83f_entry entry;
		counter += fread(&entry, 1, sizeof entry, stream);
		
		if(entry.header != 0x0B && entry.header != 0x0D) {
			error_message("Invalid entry header: %d\n", (int)entry.header);
			ti83f_release(file);
			return NULL;
		}
		if(entry.length != entry.length2) {
			error_message("Size mismatch: %d ≠ %d\n", (int)entry.length, (int)entry.length2);
			ti83f_release(file);
			return NULL;
		}
		
		file->entries = realloc(file->entries, (++file->length) * sizeof(struct ti83f_entry));
		
		file->entries[file->length - 1].type = entry.type;
		memcpy(
			file->entries[file->length - 1].name,
			entry.name,
			8);
		
		file->entries[file->length - 1].data = malloc(entry.length);
		file->entries[file->length - 1].flags = TI83F_AUTORELEASE;
		file->entries[file->length - 1].size = entry.length;
		counter += fread(file->entries[file->length - 1].data, 1, entry.length, stream);
		
		if(entry.flags & 0x80) {
			file->entries[file->length - 1].flags |= TI83F_ARCHIVED;
		}
	}
	
	uint16_t checksumFile;
	fread(&checksumFile, 1, sizeof checksumFile, stream);
	
	printf("Checksum: %04X\n", (int)checksumFile);
	
	return file;
}


struct ti83f_file * ti83f_new(uint32_t length)
{
	if(length == 0) {
		return NULL;
	}
	struct ti83f_file * file = malloc(sizeof(struct ti83f_file));
	memset(file->comment, 0, sizeof file->comment);
	
	file->length = length;
	file->entries = malloc(length * sizeof(struct ti83f_entry));
	
	for(uint32_t i = 0; i < length; i++)
	{
		memset(&file->entries[i], 0, sizeof(struct ti83f_entry));
	}
	
	return file;
}

void ti83f_release(struct ti83f_file * file)
{
	if(file == NULL) {
		return;
	}
	
	if(file->entries != NULL)
	{
		for(uint32_t i = 0; i < file->length; i++)
		{
			if(file->entries[i].data == NULL) {
				continue;
			}
			if(file->entries[i].flags & TI83F_AUTORELEASE) {
				free(file->entries[i].data);
			}
		}
	}
	
	free(file->entries);
	free(file);
}