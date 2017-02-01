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

static uint16_t calculateChecksum(void const * mem, size_t len)
{
	uint16_t cs = 0;
	uint8_t const * data = mem;
	for(size_t i = 0; i < len; i++) {
		cs += data[i];
	}
	return cs;
}

static int freadcs(void * ptr, size_t a, size_t b, FILE * f, uint16_t * cs)
{
	int r = fread(ptr, a, b, f);
	*cs += calculateChecksum(ptr, r);
	return r;
}

static int fwritecs(void * ptr, size_t a, size_t b, FILE * f, uint16_t * cs)
{
	int r = fwrite(ptr, a, b, f);
	*cs += calculateChecksum(ptr, r);
	return r;
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
	
	uint16_t checksum = 0;
	for(int counter = 0; counter < header.length; )
	{
		struct raw_ti83f_entry entry;
		counter += freadcs(&entry, 1, sizeof entry, stream, &checksum);
		
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
		counter += freadcs(
			file->entries[file->length - 1].data, 
			1, entry.length, 
			stream,
			&checksum);
		
		if(entry.flags & 0x80) {
			file->entries[file->length - 1].flags |= TI83F_ARCHIVED;
		}
	}
	
	uint16_t checksumFile;
	fread(&checksumFile, 1, sizeof checksumFile, stream);
	
	if(checksum != checksumFile) 
	{
		error_message("Checksum mismatch: %04X ≠ %04X\n", checksum, checksumFile);
	}
	
	return file;
}

void ti83f_store(FILE * stream, struct ti83f_file const * file)
{
	if(stream == NULL) {
		return;
	}
	if(file == NULL) {
		return;
	}
	
	struct raw_ti83f header;
	memcpy(header.sig, signature, sizeof signature);
	memset(header.comment, 0, sizeof header.comment);
	strncpy(header.comment, file->comment, 42);
	
	header.length = 0;
	for(uint32_t i = 0; i < file->length; i++)
	{
		if(file->entries[i].data == NULL || file->entries[i].size == 0) {
			continue;
		}
		header.length += sizeof(struct raw_ti83f_entry);
		header.length += file->entries[i].size;
	}
	
	if(header.length == 0) {
		error_message("No data to write\n");
		return;
	}
	
	fwrite(&header, 1, sizeof header, stream);
	
	uint16_t checksum = 0;
	for(uint32_t i = 0; i < file->length; i++)
	{
		if(file->entries[i].data == NULL || file->entries[i].size == 0) {
			continue;
		}
		
		struct ti83f_entry * e = &file->entries[i];
		struct raw_ti83f_entry entry;
		
		entry.header = 0x0D;
		entry.length = e->size;
		entry.type = e->type;
		memcpy(entry.name, e->name, 8);
		entry.version = 0;
		entry.flags = (e->flags & TI83F_ARCHIVED) ? 0x80 : 0x00;
		entry.length2 = e->size;
		
		fwritecs(&entry, 1, sizeof entry, stream, &checksum);
	
		fwritecs(e->data, 1, e->size, stream, &checksum);
	}
	
	fwrite(&checksum, 1, sizeof checksum, stream);
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



/*
// Test routine, should output a binary equivalent file.
int main(int argc, char ** argv)
{
	FILE * f = fopen("run.8xp", "rb");
	
	struct ti83f_file * file = ti83f_load(f);
	if(file != NULL) {
		LOG("Success!\n");
	} else {
		LOG("Failure!\n");
	}
	fclose(f);
	
	f = fopen("run2.8xp", "wb");
	ti83f_store(f, file);
	fclose(f);
	
	ti83f_release(file);
}
*/