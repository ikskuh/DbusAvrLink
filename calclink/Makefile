
# GENLISTING=1

ifdef GENLISTING
ASFLAGS=-T
else
ASFLAGS=
endif

all: send.8xp receive.8xp

%.8xp: %.asm
	spasm $(ASFLAGS) $< $@

oncalc: send.8xp receive.8xp
	ti-send -s send.8xp
	ti-send -s receive.8xp
	
.PHONY: oncalc