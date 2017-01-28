#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define NULL 0

#define USE_BINARY_PROTOCOL
// #define USE_SEND_ECHO
#define SPY_ON_WIRE

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

unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );
bool USART_CanRead(void);


#define BAUD 9600UL      // Baudrate

// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.

#if ((BAUD_ERROR<980) || (BAUD_ERROR>1020))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif

#define RED   2
#define WHITE 1

static uint8_t canReceive()
{
	return (PINA & 3) != 3;
}

static uint8_t receive(uint8_t * success)
{
	uint8_t result = 0;
	if(success) *success = 1;
	for(uint8_t i = 0; i < 8; i++)
	{
		while((PINA & 3) == 3);
		
		if((PINA & 3) == 0)
		{
			// transmit error
			if(success) *success = 0;
			while((PINA & 3) != 3);
			return 0xFF;
		}
		
		if((PINA & RED) == 0)
		{
			result |= (1<<i);
#ifndef SPY_ON_WIRE
			DDRA = WHITE;
#endif
			while((PINA & RED) == 0);
		}
		else
		{
#ifndef SPY_ON_WIRE
			DDRA = RED;
#endif
			while((PINA & WHITE) == 0);
		}
#ifndef SPY_ON_WIRE
		DDRA = 0;
#endif
		while((PINA & 3) != 3);
	}
	return result;
}

#ifdef SPY_ON_WIRE
static void send(uint8_t value) { (void)value; }
#else
static void send(uint8_t value)
{
#ifdef USE_SEND_ECHO
#ifndef USE_BINARY_PROTOCOL
	USART_Transmit('>');
	USART_Transmit(("0123456789ABCDEF")[(value >> 4) & 0xF]);
	USART_Transmit(("0123456789ABCDEF")[(value >> 0) & 0xF]);
	USART_Transmit('\n');
	USART_Transmit('\r');
#else
	USART_Transmit(value);
#endif
#endif

	for(uint8_t i = 0; i < 8; i++)
	{
		if(value & (1<<i))
		{
			DDRA = RED;
			while((PINA & WHITE) != 0); // wait for low
		}
		else
		{
			DDRA = WHITE;
			while((PINA & RED) != 0); // wait for low
		}
		DDRA = 0;
		while((PINA & 3) != 3);
	}
}
#endif

static void sendpacket(
	uint8_t machineId, 
	uint8_t commandId,
	void const * data,
	uint16_t length)
{
	send(machineId);
	send(commandId);
	if(data != NULL) {
		send(length);
		send(length >> 8);
		uint8_t const * ptr = (uint8_t const *)data;
		uint16_t checksum = 0;
		for(uint16_t i = 0; i < length; i++) {
			send(ptr[i]);
			checksum += ptr[i];
		}
		send(checksum);
		send(checksum >> 8);
	
	} else {
		send(0x0B);
		send(0x00);
	}
}

int main()
{
  // Timer 0 konfigurieren
  TCCR0 = (1<<CS01); // Prescaler 8

  // Overflow Interrupt erlauben
  TIMSK |= (1<<TOIE0);
	
	// Senden AN
  UCSRB |= (1 << TXEN) | (1 << RXEN)|(1<<RXCIE);
	
	// Async, 8N1
	UCSRC = (1<<URSEL)|(1 << UCSZ1)|(1 << UCSZ0);

	UBRRL = (UBRR_VAL >> 0) & 0xFF;
	UBRRH = (UBRR_VAL >> 8) & 0xFF;
	
	DDRB = 3;
	PORTA = 0;
	DDRA = 0;
	
	sei();
	
	uint8_t value = 0;
	uint8_t success = 0;

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
	
	uint8_t rstate = 0;
	uint8_t tmvalue = 0;
	
	while(1)
	{
		if(canReceive()) {
			uint8_t value = receive(&success);
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
			send(val);
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
						send(value);
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
	PORTB = ~(PINA & 3);
}

static volatile uint8_t rxbuffer[256];
static volatile uint8_t txbuffer[256];
static volatile uint8_t rx_read = 0;
static volatile uint8_t rx_write = 0;
static volatile uint8_t tx_read = 0;
static volatile uint8_t tx_write = 0;

ISR(USART_RXC_vect)
{
	rxbuffer[(rx_write++) % sizeof(rxbuffer)] = UDR;
}

ISR(USART_UDRE_vect)
{
	UDR = txbuffer[(tx_read++) % sizeof(txbuffer)];
	if (tx_read == tx_write) {
		UCSRB &= ~(1 << UDRIE);
	}
}

void USART_Transmit( unsigned char data )
{
	txbuffer[(tx_write++) % sizeof(txbuffer)] = data;
	UCSRB |= (1 << UDRIE);
	
	/*
	// Wait for empty transmit buffer 
	while ( !( UCSRA & (1<<UDRE)) ;
	// Put data into buffer, sends the data
	UDR = data;
	*/
}

unsigned char USART_Receive( void )
{
	while(rx_read == rx_write);
	
	return rxbuffer[(rx_read++) % sizeof(rxbuffer)];
	
	/*
	// Wait for data to be received
	while ( !(UCSRA & (1<<RXC)) );
	// Get and return received data from buffer
	return UDR;
	*/
}

bool USART_CanRead(void)
{
	return rx_read != rx_write;
	// return (UCSRA & (1<<RXC));
}