TTY=/dev/ttyUSB0
MCU=atmega32
FREQ=20000000

FLAGS=-Wall -O3 -mmcu=$(MCU) -DF_CPU=$(FREQ)LL -I../include

all: test.elf


test.elf: test.o usart.o dbus.o
	avr-gcc $(FLAGS) -o $@ $^
	avr-size -C --mcu=$(MCU) $@

%: %.hex
	
%.hex: %.elf
	avr-objcopy $< -O ihex $@

%.elf: %.o
	avr-gcc $(FLAGS) -o $@ $^
	avr-size -C --mcu=$(MCU) $@

%.o: %.c
	avr-gcc $(FLAGS) -std=c11 -c -o $@ $<

%.o: %.cpp
	avr-g++ $(FLAGS) -std=c++11 -c -o $@ $<

%\:flash: %.hex
	avrdude -p $(MCU) -c stk500v2 -P $(TTY) -e -U flash:w:$<