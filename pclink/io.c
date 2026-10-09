#include "io.h"
#include "debug.h"
#include "tty.h"
#include <stdlib.h>
#include <unistd.h>
#include <libusb.h>
#include <libusb-1.0/libusb.h>

static int serialPort = -1;
static libusb_context *ctx = NULL;
static libusb_device_handle *dev = NULL;

void sendACK()
{
	sendPacket(0x73, 0x56, NULL, 0);
}

void receiveACK()
{
	struct packet packet;
	if(receivePacket(&packet) == false) {
		error_message("Failed to receive packet.\n");
		exit(EXIT_FAILURE);
	}
	if(packet.command != 0x56) {
		error_message("Did not receive ACK, but %02X.\n", (int)packet.command);
		if(packet.data) {
			free(packet.data);
		}
		exit(EXIT_FAILURE);
	}
}

static char const * fmt_usb_error(int rc)
{
	switch(rc)
	{
		case LIBUSB_SUCCESS: return "SUCCESS";
		case LIBUSB_ERROR_IO: return "ERROR_IO";
		case LIBUSB_ERROR_INVALID_PARAM: return "ERROR_INVALID_PARAM";
		case LIBUSB_ERROR_ACCESS: return "ERROR_ACCESS";
		case LIBUSB_ERROR_NO_DEVICE: return "ERROR_NO_DEVICE";
		case LIBUSB_ERROR_NOT_FOUND: return "ERROR_NOT_FOUND";
		case LIBUSB_ERROR_BUSY: return "ERROR_BUSY";
		case LIBUSB_ERROR_TIMEOUT: return "ERROR_TIMEOUT";
		case LIBUSB_ERROR_OVERFLOW: return "ERROR_OVERFLOW";
		case LIBUSB_ERROR_PIPE: return "ERROR_PIPE";
		case LIBUSB_ERROR_INTERRUPTED: return "ERROR_INTERRUPTED";
		case LIBUSB_ERROR_NO_MEM: return "ERROR_NO_MEM";
		case LIBUSB_ERROR_NOT_SUPPORTED: return "ERROR_NOT_SUPPORTED";
		case LIBUSB_ERROR_OTHER: return "ERROR_OTHER";
		default: return "<unknown>";
	}
}

static void sendData(void const * data, uint16_t length)
{
	unsigned char const * ptr = data;
	while(length > 0)
	{
		int len;
		if(dev != NULL)
		{
			int rc = libusb_bulk_transfer(
				dev,
				0x02,            // OUT endpoint
				ptr,
				length,
				&len,
				5000             // timeout in ms
			);
			if(len < 0) {
				error_message("Failed to write stuff from usb=%s\n", fmt_usb_error(rc));
				return;
			}
		}
		else
		{
			len = write(serialPort, data, length);
			if(len < 0) {
				error_message("Failed to write to serial port: %d\n", len);
				return;
			}
		}
		if(len <= 0)
			return;
		ptr += len;
		length -= len;
	}
}

void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length)
{
	sendData(&machineId, 1);
	sendData(&command, 1);
	sendData(&length, 2);
	if(data != NULL) {
		sendData(data, length);
		uint16_t cs = calculateCheckSum(data, length);
		sendData(&cs, 2);
	}
}

static void readabs(void * buffer, size_t nBytes)
{
	uintptr_t ptr = (uintptr_t)buffer;
	for(size_t i = 0; i < nBytes;)
	{
		int len;
		if(dev != NULL)
		{
			int rc = libusb_bulk_transfer(
				dev,
				0x81,            // IN endpoint
				(void*)ptr,
				nBytes - i,
				&len,
				5000
			);
			if(rc != 0)
			{
				error_message("Failed to read stuff from usb=%s\n", fmt_usb_error(rc));
				return;
			}
			// TODO: Check error on rc
		}
		else
		{
			len = read(serialPort, (void*)ptr, nBytes - i);
			if(len == -1) {
				error_message("Failed to read stuff from fd=%d\n", serialPort);
				return;
			}
		}
		ptr += len;
		i += len;
	}
}

bool receivePacket(struct packet * packet)
{
	readabs(&packet->machine, 1);
	readabs(&packet->command, 1);
	readabs(&packet->length, 2);
	switch(packet->command) {
		case 0x06:
		case 0x15:
		case 0x36:
		case 0x88:
		case 0xA2:
		case 0xC9:
		{
			packet->data = malloc(packet->length);
			readabs(packet->data, packet->length);
			uint16_t cs0 = calculateCheckSum(packet->data, packet->length);
			readabs(&packet->checksum, 2);
			if(cs0 != packet->checksum) {
				error_message("Invalid Checksum: %04X ≠ %04X\n", packet->checksum, cs0);
			}
			return (cs0 == packet->checksum);
		}
		default:
		{
			packet->length = 0;
			packet->data = 0;
			packet->checksum = 0xFFFF;
			return true;
		}
	}
}


uint16_t calculateCheckSum(void const * data, uint16_t length)
{
	uint16_t checksum = 0;
	uint8_t const * _data  = data;
	for(int i = 0; i < length; i++) {
		checksum += _data[i];
	}
	return checksum;
}


bool io_open_serial(char const * portName, int baud)
{	
	io_close();

	serialPort = ttyOpen (portName, baud);
	if(serialPort < 0) {
		error_message("Failed to open serial port %s\n", portName);
		return false;
	}
	return true;
}

bool io_open_usb(int vid, int pid)
{
	io_close();

	libusb_init(&ctx);
	if(ctx == NULL) {
		return false;
	}

	dev = libusb_open_device_with_vid_pid(
			ctx, vid, pid
	);
	if(dev == NULL) {
		libusb_exit(ctx);
		ctx = NULL;
		return false;
	}

	// TODO: Error handling:
	libusb_claim_interface(dev, 0);

	return true;
}

void io_close(void)
{
	if(serialPort != -1)
	{
		close(serialPort);
		serialPort = -1;
	}
	if(dev != NULL)
	{
		libusb_close(dev);
		dev = NULL;

		libusb_exit(ctx);
		ctx = NULL;
	}
}