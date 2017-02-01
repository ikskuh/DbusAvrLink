# D-BUS–AVR–Link

This project provides a basic implementation for the D-BUS protocol
that is used by Texas Instruments Calculates like the *TI-83+*.

## D-Bus Protocol

The protocol is a simple, half-duplex serial protocol which allows
basic control flow like error transmission and a simple "request to send"
facility. To read more about the protocol, search for *Link Protocol Guide*
at the bottom of this page.

## Implementation
The current implementation provides two modes of operation:

- Active Transceiver
- Passive "spy-on-wire" mode

Each mode of operation is a synchronous mode with which only blocking
communication can be achieved.

## `pclink` Toolset
The folder `pclink` contains some tools that implement a basic
calculator communication toolset for sending and receiving data
as well as transmitting programs in *Silent Mode* (planned).

### `send`
**Planned:** `send` will transfer one or more files to the calculator.

### `receive`
**Planned:** `receive` will receive one or more transmissions from the
calculator and store them into a file.

### `ls`
**Planned:** `ls` will list the calculators directory contents.

## See Also

- [Link Protocol Guide](http://merthsoft.com/linkguide/index.html)
- [Learn TI-83 Plus Assembly In 28 Days](http://tutorials.eeems.ca/ASMin28Days/welcome.html)
- [ATmega32 Datasheet](http://www.atmel.com/Images/doc2503.pdf)

## TODO
The project isn't completed yet, the target is to create a C and C++ API
for the D-Bus protocol with synchronous and asynchronous IO.

- Create asynchronous C API
	- Create a timer-based approach for asynchronous IO
- Create protocol objects for C++ that allows multiple communcation objects
	- Synchronous communication
	- Asynchronous communcation