#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>

#define error_message(...) fprintf(stderr, __VA_ARGS__)

#define VERBOSE

int set_interface_attribs (int fd, int speed, int parity);

void set_blocking (int fd, int should_block);

int serialPort;

struct packet
{
	uint8_t machine;
	uint8_t command;
	uint16_t length;
	void * data;
	uint16_t checksum;
};

struct varheader
{
	uint16_t size;
	uint8_t type;
	uint8_t name[8];
	uint8_t version;
	uint8_t type2;
} __attribute__((packed));

uint16_t calculateCheckSum(void const * data, uint16_t length);

void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length);

static void readabs(int fd, void * buffer, size_t nBytes);

bool receivePacket(struct packet * packet);

void extractVarHdr(struct packet const * packet, struct varheader * header);

void dumpHeader(struct varheader const * header);

void dumpPacket(struct packet const * packet);

void dumpSafeString(char const * str, uint16_t len);

void detokenize(char * dst, char const * src, int len);

int main(int argc, char ** argv)
{
	char const * portName = "/dev/ttyUSB1";
	
	serialPort = open (portName, O_RDWR | O_NOCTTY | O_SYNC);
	if (serialPort < 0)
	{
		error_message ("error %d opening %s: %s\n", errno, portName, strerror (errno));
		return 1;
	}

	set_interface_attribs (serialPort, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (serialPort, 1);
	
	bool success;
	struct packet packet;
	
	printf("Waiting...\n");
	
	
	
	while(true)
	{
		success = receivePacket(&packet);
		
#ifdef VERBOSE
		printf("Packet:\n");
		dumpPacket(&packet);
		switch(packet.command) {
			case 0x06:
			case 0x88:
			case 0xA2:
			case 0xC9:
				printf("Var Header:\n");
				dumpHeader(packet.data);
				break;
		}
#endif
		
		if(!success) {
			error_message("Received packet with invalid checksum!\n");
			break; // stop on invalid data
		}
			
		switch(packet.command)
		{
			case 0x68: // CHECK RDY
				// Let's say we are an TI83+ :)
				sendPacket(0x73, 0x56, NULL, 0);
				break;
			case 0x06: // Var Header
			{
				printf("Got VAR HDR, now ACK, CTS\n");
				
				dumpHeader(packet.data);
				
				// Accept the variable header by saying:
				// ACK, CTS
				sendPacket(0x73, 0x56, NULL, 0);
				sendPacket(0x73, 0x09, NULL, 0);
				break;
			}
			case 0x92: // End Of transmittion
			{
				// ACK this shit
				sendPacket(0x73, 0x56, NULL, 0);
				printf("Done.\n");
				break; 
			}
			case 0x15: // DATA
			{
				printf("File Contents:\n");
				
				uint8_t * const data = packet.data;
				for(int i = 0; i < packet.length; i++) {
					printf(" %02X", data[i]);
				}
				printf("\n");
				
				printf("'");
				dumpSafeString(packet.data, packet.length);
				printf("'\n");
				
				static char msg[2048];
				
				detokenize(msg, (char*)packet.data + 2, packet.length - 2);
				
				printf("'");
				printf("%s", msg);
				printf("'\n");
			
				// ACK Packet
				sendPacket(0x73, 0x56, NULL, 0);
				break;
			}
			case 0x56: // ACK
			{
				// we are happy with simple acknowledge
				break;
			}
			default:
				error_message("Invalid command received: %02X", packet.command);
				break;
		}
		
		if(packet.data) free(packet.data);
	}
	
	close(serialPort);
}


int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		error_message ("error %d from tcgetattr\n", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
														// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
														// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		error_message ("error %d from tcsetattr\n", errno);
		return -1;
	}
	return 0;
}

void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		error_message ("error %d from tggetattr\n", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		error_message ("error %d setting term attributes\n", errno);
}


void sendPacket(uint8_t machineId, uint8_t command, void const * data, uint16_t length)
{
	write(serialPort, &machineId, 1);
	write(serialPort, &command, 1);
	write(serialPort, &length, 2);
	if(data != NULL) {
		write(serialPort, data, length);
		uint16_t cs = calculateCheckSum(data, length);
		write(serialPort, &cs, 2);
	}
}

static void readabs(int fd, void * buffer, size_t nBytes)
{
	uintptr_t ptr = (uintptr_t)buffer;
	for(size_t i = 0; i < nBytes;)
	{
		int len = read(fd, (void*)buffer, nBytes - i);
		if(len == -1) {
			error_message("Failed to read stuff from fd=%d\n", fd);
			return;
		}
		buffer += len;
		i += len;
	}
}

bool receivePacket(struct packet * packet)
{
	readabs(serialPort, &packet->machine, 1);
	readabs(serialPort, &packet->command, 1);
	readabs(serialPort, &packet->length, 2);
	switch(packet->command) {
		case 0x06:
		case 0x15:
		case 0x36:
		case 0x88:
		case 0xA2:
		case 0xC9:
		{
			packet->data = malloc(packet->length);
			readabs(serialPort, packet->data, packet->length);
			uint16_t cs0 = calculateCheckSum(packet->data, packet->length);
			readabs(serialPort, &packet->checksum, 2);
			
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

void extractVarHdr(
	struct packet const * packet,
	struct varheader * header)
{
	memset(header, 0, sizeof *header);
	memcpy(header, packet->data, packet->length);
}

void dumpHeader(struct varheader const * header)
{
	printf("SIZ = %04X\n", header->size);
	printf("TYP = %02X\n", header->type);
	printf("NAM =");
	for(int i = 0; i < 8; i++) {
		printf(" %02X", header->name[i]);
	}
	
	printf(" \"");
	static char name[128];
	detokenize(name, header->name, 8);
	printf("%s", name);
	// dumpSafeString(header->name, 8);
	printf("\"\n");
	printf("VER = %02X\n", header->version);
	printf("TP2 = %02X\n", header->type2);
}

void dumpPacket(struct packet const * packet)
{
	printf("MID = %02X\n", packet->machine);
	printf("CMD = %02X\n", packet->command);
	printf("LEN = %04X\n", packet->length);
	if(packet->data != NULL) {
		printf("DAT =");
		uint8_t * const data = packet->data;
		for(int i = 0; i < packet->length; i++) {
			printf(" %02X", data[i]);
		}
		printf("\n");
	} else {
		printf("DAT = NULL\n");
	}
	printf("CHK = %04X\n", packet->checksum);
}

void dumpSafeString(char const * str, uint16_t len)
{
	for(int i = 0; i < len; i++) {
		char c = str[i];
		if(c >= 0x20 && c < 127) {
			printf("%c", c);
		} else {
			printf(".");
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



static char const * normalTokens[256] = 
{
	"", // 0
	">DMS", // 1
	">DEC", // 2
	">FRAC", // 3
	"→", // 4
	"BoxPlot", // 5
	"[", // 6
	"]", // 7
	"{", // 8
	"}", // 9
	"ʳ", // 10
	"°", // 11
	"⁻¹", // 12
	"²", // 13
	"ᵗ", // 14
	"³", // 15
	"(", // 16
	")", // 17
	"round(", // 18
	"pxl-Test(", // 19
	"augment(", // 20
	"rowSwap(", // 21
	"row+(", // 22
	"*row(", // 23
	"*row+(", // 24
	"max(", // 25
	"min(", // 26
	"R>Pr(", // 27
	"R>Pθ(", // 28
	"P>Rx(", // 29
	"P>Ry(", // 30
	"median(", // 31
	"randM(", // 32
	"mean(", // 33
	"solve(", // 34
	"seq(", // 35
	"fnInt(", // 36
	"nDeriv(", // 37
	"", // 38
	"fMin(", // 39
	"fMax(", // 40
	"_", // 41
	"\"", // 42
	",", // 43
	"ⅈ", // 44
	"!", // 45
	"CubicReg ", // 46
	"QuartReg ", // 47
	"0", // 48
	"1", // 49
	"2", // 50
	"3", // 51
	"4", // 52
	"5", // 53
	"6", // 54
	"7", // 55
	"8", // 56
	"9", // 57
	".", // 58
	"ₑ", // 59
	" or ", // 60
	" xor ", // 61
	":", // 62
	"\n", // 63
	" and ", // 64
	"A", // 65
	"B", // 66
	"C", // 67
	"D", // 68
	"E", // 69
	"F", // 70
	"G", // 71
	"H", // 72
	"I", // 73
	"J", // 74
	"K", // 75
	"L", // 76
	"M", // 77
	"N", // 78
	"O", // 79
	"P", // 80
	"Q", // 81
	"R", // 82
	"S", // 83
	"T", // 84
	"U", // 85
	"V", // 86
	"W", // 87
	"X", // 88
	"Y", // 89
	"Z", // 90
	"θ", // 91
	"›5C‹", // 92
	"›5D‹", // 93
	"›5E‹", // 94
	"prgm", // 95
	"›60‹", // 96
	"›61‹", // 97
	"›62‹", // 98
	"›63‹", // 99
	"Radian", // 100
	"Degree", // 101
	"Normal", // 102
	"Sci", // 103
	"Eng", // 104
	"Float", // 105
	"=", // 106
	"<", // 107
	">", // 108
	"≤", // 109
	"≥", // 110
	"≠", // 111
	"+", // 112
	"-", // 113
	"Ans", // 114
	"Fix ", // 115
	"Horiz", // 116
	"Full", // 117
	"Func", // 118
	"Param", // 119
	"Polar", // 120
	"Seq", // 121
	"IndpntAuto", // 122
	"IndpntAsk", // 123
	"DependAuto", // 124
	"DependAsk", // 125
	"›7E‹", // 126
	"☐", // 127
	"+", // 128???
	"⁺", // 129
	"×", // 130
	"/", // 131
	"Trace", // 132
	"ClrDraw", // 133
	"ZStandard", // 134
	"ZTrig", // 135
	"ZBox", // 136
	"Zoom In", // 137
	"Zoom Out", // 138
	"ZSquare", // 139
	"ZInteger", // 140
	"ZPrevious", // 141
	"ZDecimal", // 142
	"ZoomStat", // 143
	"ZoomRcl", // 144
	"PrintScreen", // 145
	"ZoomSto", // 146
	"Text(", // 147
	" nPr ", // 148
	" nCr ", // 149
	"FnOn ", // 150
	"FnOff ", // 151
	"StorePic ", // 152
	"RecallPic ", // 153
	"StoreGDB ", // 154
	"RecallGDB ", // 155
	"Line(", // 156
	"Vertical ", // 157
	"Pt-On(", // 158
	"Pt-Off(", // 159
	"Pt-Change(", // 160
	"Pxl-On(", // 161
	"Pxl-Off(", // 162
	"Pxl-Change(", // 163
	"Shade(", // 164
	"Circle(", // 165
	"Horizintal ", // 166
	"Tangent(", // 167
	"DrawInv ", // 168
	"DrawF ", // 169
	"›AA‹", // 170
	"rand", // 171
	"π", // 172
	"getKey", // 173
	"'", // 174
	"?", // 175
	"⁻", // 176
	"int(", // 177
	"abs(", // 178
	"det(",
	"identity(",
	"dim(",
	"sum(",
	"prod(",
	"not(",
	"iPart(",
	"fPart(",
	"›BB‹",
	"√(",
	"³√(",
	"ln(",
	"e^(",
	"log(",
	"₁₀^(",
	"sin(", // 179
	"sin⁻¹(", // 180
	"cos(", // 181
	"cos⁻¹(", // 182
	"tan(", // 183
	"tan⁻¹(", // 184
	"sinh(", // 185
	"sinh⁻¹(", // 186
	"cosh(", // 187
	"cosh⁻¹(", // 188
	"tanh(", // 189
	"tanh⁻¹(", // 190
	"If ", // 191
	"Then", // 192
	"Else", // 193
	"While ", // 194
	"Repeat ", // 195
	"For(", // 196
	"End", // 197g
	"Return", // 198
	"Lbl ", // 199
	"Goto ", // 200
	"Pause ", // 201
	"Stop", // 202
	"IS>(", // 203
	"DS>(", // 204
	"Input ", // 205
	"Prompt ", // 206
	"Disp ", // 207
	"DispGraph", // 208
	"Output(", // 209
	"ClrHome", // 210
	"Fill(", // 211
	"SortA(", // 212
	"SortD(", // 213
	"DispTable", // 214
	"Menu(", // 215
	"Send(", // 216
	"Get(", // 217
	"PlotsOn ", // 218
	"PlotsOff ", // 219
	"ₗ", // 220
	"Plot1(", // 221
	"Plot2(", // 222
	"Plot3(", // 223
	"", // 224
	"^", // 225
	"*√", // 226
	"1-Var Stats ", // 227
	"2-Var Stats ", // 228
	"LinReg(a+bx) ", // 229
	"ExpReg ", // 230
	"LnReg ", // 231
	"PwrReg ", // 232
	"Med-Med ", // 233
	"QuadReg ", // 234
	"ClrList ", // 235
	"ClrTable", // 236
	"Histogramm", // 237
	"xyLine", // 238
	"Scatter", // 239
	"LinReg(ax+b)", // 240", // 255
};

void detokenize(char * dst, char const * src, int len)
{
	static char temp[64];
	for(int i = 0; i < len; i++)
	{
		char const * tok = NULL;
		switch((uint8_t)*src)
		{
			case 0:
				*dst++ = 0;
				return;
			case 0x5C:
				sprintf(temp, "[%c]", 'A' + *src++);
				tok = temp;
				break;
			case 0x5D:
				switch(*(++src)) {
					case 0: tok = "L₁"; break;
					case 1: tok = "L₂"; break;
					case 2: tok = "L₃"; break;
					case 3: tok = "L₄"; break;
					case 4: tok = "L₅"; break;
					case 5: tok = "L₆"; break;
					case 6: tok = "L₇"; break;
					case 7: tok = "L₈"; break;
					case 8: tok = "L₉"; break;
					case 9: tok = "L₀"; break;
					default:tok = "Lₙ"; break;
				}
				break;
			case 0x5E:
				// Ignore tokens
				break;
			case 0x60:
			case 0x61:
			case 0x62:
			case 0x63:
				break;
			case 0xAA:
				switch(*(++src)) {
					case 0: tok = "Str1"; break;
					case 1: tok = "Str2"; break;
					case 2: tok = "Str3"; break;
					case 3: tok = "Str4"; break;
					case 4: tok = "Str5"; break;
					case 5: tok = "Str6"; break;
					case 6: tok = "Str7"; break;
					case 7: tok = "Str8"; break;
					case 8: tok = "Str9"; break;
					case 9: tok = "Str0"; break;
					default:tok = "Str?"; break;
				}
				break;
			case 0xBB:
				break;
			default:
				tok = normalTokens[*src];
				break;
		}
		if(tok != NULL) {
			while(*tok) {
					*dst++ = *tok++;
			}
		}
		src++;
	}
	*dst = 0;
}