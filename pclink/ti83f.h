#pragma once

#include <stdint.h>

struct ti83f
{
	uint8_t sig[11];
	char comment[42];
	uint16_t length;
} __attribute__((packed));

struct ti83f_entry
{
	uint16_t header;
	uint16_t length;
	uint8_t type;
	uint8_t name[8];
	uint8_t version;
	uint8_t flags;
	uint16_t length2;
}  __attribute__((packed));

