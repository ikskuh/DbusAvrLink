#include "ti83plus.inc"
#define  progStart   $9D95
.org     progStart-2
.db      $BB,$6D

; API:
; Call Asm(prgmRTS)
; Get Ans with 1 if a byte can be received, else get a 0.

main:
	in a, (0)
	and $03
	cp 3
	jr NZ,_rts
	ld a,0
	jr _fini
_rts:
	ld a,1
_fini:
	bcall(_SetXXOP1)
	bcall(_StoAns)
	ret
