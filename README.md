# D-BUS–AVR–Link

This project provides a basic implementation for the D-BUS protocol
that is used by Texas Instruments Calculates like the *TI-83+*.

The protocol is a simple, half-duplex serial protocol which allows
basic control flow like error transmission and a simple "request to send"
facility. To read more about the protocol, search for *Link Protocol Guide*
at the bottom of this page.

## Implementation
The current implementation provides two modes of operation:

- Active Transceiver
- Passive "spy-on-wire" mode



## See Also

- [Link Protocol Guide](http://merthsoft.com/linkguide/index.html)
- [Learn TI-83 Plus Assembly In 28 Days](http://tutorials.eeems.ca/ASMin28Days/welcome.html)
- [ATmega32 Datasheet](http://www.atmel.com/Images/doc2503.pdf)