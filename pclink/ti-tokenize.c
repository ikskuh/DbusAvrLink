#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "debug.h"
#include "tokens.h"

extern char const * tok_tbl0xBB[];

int main(int argc, char ** argv)
{
	// Add command line options here
	
	while(!feof(stdin))
	{
		int c = fgetc(stdin);
		if(c < 0) {
			break; // End of file or error
		}
		if(tok_isTwoByte(c)) {
			int c2 = fgetc(stdin);
			if(c2 < 0) {
				break; // Still end of file or error
			}
			fputs(tok_getTwoByte(c, c2), stdout);
		} else {
			fputs(tok_getOneByte(c), stdout);
		}
	}
	fclose(stdin);
	fclose(stdout);
}