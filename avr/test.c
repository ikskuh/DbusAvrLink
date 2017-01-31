#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define NULL 0

#define USE_BINARY_PROTOCOL
// #define USE_SEND_ECHO
// #define SPY_ON_WIRE

#include "usart.h"
#include "dbus.h"

void delay_ms(unsigned time)
{
	while(time > 0)
	{
		_delay_ms(1);
		--time;
	}
}

void delay_us(unsigned time)
{
	while(time > 0)
	{
		_delay_us(1);
		--time;
	}
}


/**
 * @brief Sends a packet in DBus protocol format
 */
void sendpacket(
	uint8_t machineId, 
	uint8_t commandId,
	void const * data,
	uint16_t length)
{
	DBUS_Send(machineId);
	DBUS_Send(commandId);
	if(data != NULL) {
		DBUS_Send(length);
		DBUS_Send(length >> 8);
		uint8_t const * ptr = (uint8_t const *)data;
		uint16_t checksum = 0;
		for(uint16_t i = 0; i < length; i++) {
			DBUS_Send(ptr[i]);
			checksum += ptr[i];
		}
		DBUS_Send(checksum);
		DBUS_Send(checksum >> 8);
	
	} else {
		DBUS_Send(0x00);
		DBUS_Send(0x00);
	}
}

int main()
{
  // Timer 0 konfigurieren
  TCCR0 = (1<<CS01); // Prescaler 8

  // Overflow Interrupt erlauben
  TIMSK |= (1<<TOIE0);
	
	USART_Initialize();
	
	DBUS_Initialize();
	
	sei();
	
	bool success;

#ifndef USE_BINARY_PROTOCOL
	USART_Transmit('S');
	USART_Transmit('T');
	USART_Transmit('A');
	USART_Transmit('R');
	USART_Transmit('T');
	USART_Transmit('\n');
	USART_Transmit('\r');
#endif

	// sendpacket(0x73, 0x56, NULL, 0);
	
#ifndef USE_BINARY_PROTOCOL
	uint8_t rstate = 0;
	uint8_t value = 0;
#endif

	DDRB = (DBUS_RED | DBUS_WHITE);
	
	while(1)
	{
		if(DBUS_CanReceive()) {
			uint8_t value = DBUS_Receive(&success);
			if(success) {
#ifdef USE_BINARY_PROTOCOL
				USART_Transmit(value);
#else
				USART_Transmit('<');
				USART_Transmit(("0123456789ABCDEF")[(value >> 4) & 0xF]);
				USART_Transmit(("0123456789ABCDEF")[(value >> 0) & 0xF]);
#endif
			} else {
				USART_Transmit(0xFF);
			}
				
#ifndef USE_BINARY_PROTOCOL
			USART_Transmit('\n');
			USART_Transmit('\r');
#endif
		}
		if(USART_CanRead())
		{
			uint8_t val = USART_Receive();
#ifdef USE_BINARY_PROTOCOL
			DBUS_Send(val);
#else
			if((val >= '0' && val <= '9') || (val >= 'A' && val <= 'F'))
			{
				if(val >= '0' && val <= '9') {
					val -= '0';
				} else {
					val -= 'A';
					val += 0xA;
				}
				switch(rstate)
				{
					case 0: // Receive MSB
						value = val << 4;
						rstate = 1;
						break;
					case 1: // Receive LSB
						value |= val;
						rstate = 0;
						DBUS_Send(value);
						break;
				}
			}
#endif
		}
	}
	
	return 0;
}

ISR (TIMER0_OVF_vect)
{
	PORTB = ~PINA;
}