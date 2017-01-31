#include "dbus.h"

#define DBUS_MASK (DBUS_RED | DBUS_WHITE)

void DBUS_Initialize()
{
	DBUS_PORT &= ~DBUS_MASK;  // Set to "low"
	DBUS_DIR  &= ~DBUS_MASK; // but Input
}

bool DBUS_CanReceive()
{
	return (DBUS_PIN & DBUS_MASK) != DBUS_MASK;
}

uint8_t DBUS_Receive(bool * success)
{
	uint8_t result = 0;
	if(success) *success = true;
	DBUS_DIR &= ~DBUS_MASK;
	for(uint8_t i = 0; i < 8; i++)
	{
		while((DBUS_PIN & DBUS_MASK) == DBUS_MASK);
		
		if((DBUS_PIN & DBUS_MASK) == 0)
		{
			// transmit error
			if(success) *success = false;
			while((DBUS_PIN & DBUS_MASK) != DBUS_MASK);
			return 0xFF;
		}
		
		if((DBUS_PIN & DBUS_RED) == 0)
		{
			result |= (1<<i);
			DBUS_DIR = DBUS_WHITE;
			while((DBUS_PIN & DBUS_RED) == 0);
		}
		else
		{
			DBUS_DIR |= DBUS_RED;
			while((DBUS_PIN & DBUS_WHITE) == 0);
		}
		DBUS_DIR &= ~DBUS_MASK;
		while((DBUS_PIN & DBUS_MASK) != DBUS_MASK);
	}
	return result;
}

uint8_t DBUS_Spy(bool * success)
{
	uint8_t result = 0;
	if(success) *success = true;
	DBUS_DIR &= ~DBUS_MASK;
	for(uint8_t i = 0; i < 8; i++)
	{
		while((DBUS_PIN & DBUS_MASK) == DBUS_MASK);
		
		if((DBUS_PIN & DBUS_MASK) == 0)
		{
			// transmit error
			if(success) *success = false;
			while((DBUS_PIN & DBUS_MASK) != DBUS_MASK);
			return 0xFF;
		}
		
		if((DBUS_PIN & DBUS_RED) == 0)
		{
			result |= (1<<i);
			while((DBUS_PIN & DBUS_RED) == 0);
		}
		else
		{
			while((DBUS_PIN & DBUS_WHITE) == 0);
		}
		while((DBUS_PIN & DBUS_MASK) != DBUS_MASK);
	}
	return result;
}

bool DBUS_Send(uint8_t value)
{
	DBUS_DIR &= ~DBUS_MASK;
	if((DBUS_PIN & DBUS_MASK) != DBUS_MASK) {
		// Can't send right now, having a receive request incoming
		return false;
	}
	for(uint8_t i = 0; i < 8; i++)
	{
		if(value & (1<<i))
		{
			DBUS_DIR |= DBUS_RED;
			while((DBUS_PIN & DBUS_WHITE) != 0); // wait for low
		}
		else
		{
			DBUS_DIR |= DBUS_WHITE;
			while((DBUS_PIN & DBUS_RED) != 0); // wait for low
		}
		DBUS_DIR &= ~DBUS_MASK;
		// TODO: Error Checking here...
		while((DBUS_PIN & DBUS_MASK) != DBUS_MASK);
	}
	return true;
}
