#include "usart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.

#if ((BAUD_ERROR<980) || (BAUD_ERROR>1020))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
  #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif

static volatile uint8_t rxbuffer[256];
static volatile uint8_t txbuffer[256];
static volatile uint8_t rx_read = 0;
static volatile uint8_t rx_write = 0;
static volatile uint8_t tx_read = 0;
static volatile uint8_t tx_write = 0;

void USART_Initialize(void)
{
	
	// Senden AN
  UCSRB |= (1 << TXEN) | (1 << RXEN)|(1<<RXCIE);
	
	// Async, 8N1
	UCSRC = (1<<URSEL)|(1 << UCSZ1)|(1 << UCSZ0);

	UBRRL = (UBRR_VAL >> 0) & 0xFF;
	UBRRH = (UBRR_VAL >> 8) & 0xFF;
}

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