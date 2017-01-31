#pragma once

#include <stdbool.h>
#include <avr/io.h>

/**
 * Alias for any AVR PORT* where the DBUS is attached.
 */
#define DBUS_PORT PORTA

/**
 * Alias for any AVR DDR* where the DBUS is attached.
 */
#define DBUS_DIR  DDRA

/**
 * Alias for any AVR PIN* where the DBUS is attached.
 */
#define DBUS_PIN  PINA



/**
 * Pin mask of the pin that is connected to the red cable.
 */
#define DBUS_RED   (1<<PA4)

/**
 * Pin mask of the pin that is connected to the white cable.
 */
#define DBUS_WHITE (1<<PA6)



/**
 * Initializes the DBUS protocol handler.
 */
void DBUS_Initialize();

/**
 * Checks if a byte can be read from the dbus port.
 * @returns     true on success, false otherwise.
 */
bool DBUS_CanReceive();

/**
 * Sends a byte over the dbus port.
 * @param value The byte to be sent.
 * @returns     true on success, false otherwise.
 */
bool DBUS_Send(uint8_t value);

/**
 * Tries to receive a byte over dbus.
 * @param success A pointer to a variable that tells if the receiption was successful or not. If NULL, no result will be written.
 * @returns       The byte received.
 */
uint8_t DBUS_Receive(bool * success);

/**
 * Tries to spy a transmittion of a byte on dbus connection.
 * @param success A pointer to a variable that tells if the receiption was successful or not. If NULL, no result will be written.
 * @returns       The byte received.
 */
uint8_t DBUS_Spy(bool * success);