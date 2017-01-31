#pragma once

#include <stdio.h>
#include <stdint.h>

#include "io.h"

#define error_message(...) fprintf(stderr, __VA_ARGS__)

void dumpHeader(struct varheader const * header);

void dumpPacket(struct packet const * packet);

void dumpSafeString(char const * str, uint16_t len);