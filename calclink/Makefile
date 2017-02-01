
# GENLISTING=1

ifdef GENLISTING
ASFLAGS=-T
else
ASFLAGS=
endif

all: send.8xp receive.8xp

%.8xp: %.asm
	spasm $(ASFLAGS) $< $@