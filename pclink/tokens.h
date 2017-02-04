#pragma once

#include <stdint.h>
#include <stdbool.h>

void detokenize(char * dst, uint8_t const * src, int len);

/**
 * Checks, if a token symbol is a two-byte token.
 */
bool tok_isTwoByte(uint8_t tok);

/**
 * Returns the string representation for a two-byte token.
 */
char const * tok_getTwoByte(uint8_t first, uint8_t second);

/**
 * Return the string representation for a one-byte token.
 */
char const * tok_getOneByte(uint8_t first);