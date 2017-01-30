#pragma once

#include <stdbool.h>

#define BAUD 9600UL

void USART_Initialize(void);

unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );
bool USART_CanRead(void);