#include "ti83plus.inc"
#define  progStart   $9D95
.org     progStart-2
.db      $BB,$6D

; API:
; Put Real,List or String into Ans
; Call Asm(prgmSEND)
; Get the number of sent bytes in Ans


; Uncomment this for printing the byte values instead of sending them.
; #define SENDSTUB 1
	
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
