#include "ti83plus.inc"
#define  progStart   $9D95
.org     progStart-2
.db      $BB,$6D

; Planned API:
; Put number of bytes to read in Ans, maximum are 255
; Call Asm(prgmRECEIVE)
; Get the received bytes as a list in Ans

main:
	ld hl, message
	bcall(_PutS)
	ret

message:
	.db "NOT IMPLEMENTED!", 0