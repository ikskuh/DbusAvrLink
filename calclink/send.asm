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
	; Load the Ans name and find the variable
	bcall(_AnsName)
	bcall(_FindSym)
	jr NC, _found
	
	bcall(_ErrUndefined)
	
_found:
	; Filter out garbage of the file type,
	and a,$1F
	
	; switch over a (var type)
	
	; case RealObj
	cp RealObj
	jp Z,isReal
	
	; case ListObj
	cp ListObj
	jp Z,isList
	
	; case StrngObj
	cp StrngObj
	jp Z,isString
	
	bcall(_ErrDataType)

isReal:

	ld h,d
	ld l,e
	bcall(_Mov9ToOP1)
	
	bcall(_ConvOP1)
	
	call sendByte
	cp 0
	jp Z,finalize
	
	call incSent
	
	jp finalize
	
isList:
	ld h,d
	ld l,e
	
	; Read list length into bc
	ld c,(hl)
	inc hl
	ld b,(hl)
	inc hl
isList_loop:
	ld a,c
	cp 0
	jr NZ,isList_cont
	ld a,b
	cp 0
	jr Z,isList_stop
isList_cont:
	dec bc
	push hl
	push bc
	
	bcall(_Mov9ToOP1)
	bcall(_ConvOP1)
	
	call sendByte
	
	pop bc
	pop hl
	
	cp 0
	jr Z, isList_stop
	
	; hl += 9
	inc hl
	inc hl
	inc hl
	inc hl
	inc hl
	inc hl
	inc hl
	inc hl
	inc hl
	
	push hl
	call incSent
	pop hl
	
	jr isList_loop
isList_stop:
	jp finalize
	
isString:
	; Try to print the string...
	ld h,d
	ld l,e
	
	ld c,(hl)
	inc hl
	ld b,(hl)
	inc hl
isString_loop:
	ld a,c
	cp 0
	jr NZ,isString_cont
	ld a,b
	cp 0
	jr Z,isString_stop
isString_cont:
	ld a, (hl)
	
	push hl
	push bc
	
	call sendByte
		
	pop bc
	pop hl
	
	cp 0
	jr Z, isString_stop
	
	push hl
	call incSent
	pop hl
	
	dec bc
	inc hl
	
	jr isString_loop
isString_stop:
	jp finalize


; sets ans according to failStatus, then quits
finalize:
	ld hl,bytesSent
	ld a,(hl)
	bcall(_SetXXOP1)
	bcall(_StoAns)
	ret

quitWithMessage:
puts:
	bcall(_PutS)
	bcall(_NewLine)
	ret

; a sendByte(uint8_t a) { try { sendAByte(); return 1; } catch { return 0; } }
; affects:  failStatus
; destroys: all
sendByte:
#ifndef SENDSTUB
	AppOnErr(failByte)
	bcall(_SendAByte)
	AppOffErr
	ld a,(failStatus)
#else
	AppOnErr(failByte)
	ld h,0
	ld l,a
	bcall(_DispHL)
	bcall(_NewLine)
	AppOffErr
	ld a,1
#endif
	ret

; void failByte() { failStatus = 0; }
; destroys: a
; affects:  failStatus
failByte:
	ld a, 0
	ld (failStatus),0
	ret

; void incSent() { bytesSent++; }
; destroys: a
; affects:  bytesSent
incSent:
	ld a,(bytesSent)
	inc a
	ld (bytesSent),a
	ret

bytesSent:  ; Number of bytes sent.
	.db 0
	
failStatus: ; If 0, the sending of a byte has failed
	.db 1
	
msgNotFound:
	.db "NOT FOUND!", 0
msgNotSupported:
	.db "UNSUPPORTED", 0