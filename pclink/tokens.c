#include "tokens.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

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

void detokenize(char * dst, uint8_t const * src, int len)
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
				sprintf(temp, "[%c]", 'A' + (*(++src)));
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
				switch(*(++src)) {
					case 0x10: tok="Y₁"; break;
					case 0x11: tok="Y₂"; break;
					case 0x12: tok="Y₃"; break;
					case 0x13: tok="Y₄"; break;
					case 0x14: tok="Y₅"; break;
					case 0x15: tok="Y₆"; break;
					case 0x16: tok="Y₇"; break;
					case 0x17: tok="Y₈"; break;
					case 0x18: tok="Y₉"; break;
					case 0x19: tok="Y₀"; break;
					
					// Insert more here...
					
					
					default:tok="??"; break;
				}
				break;
			case 0x60:
				switch(*(++src)) {
					case 0: tok = "Pic1"; break;
					case 1: tok = "Pic2"; break;
					case 2: tok = "Pic3"; break;
					case 3: tok = "Pic4"; break;
					case 4: tok = "Pic5"; break;
					case 5: tok = "Pic6"; break;
					case 6: tok = "Pic7"; break;
					case 7: tok = "Pic8"; break;
					case 8: tok = "Pic9"; break;
					case 9: tok = "Pic0"; break;
					default:tok = "Pic?"; break;
				}
				break;
			case 0x61:
				switch(*(++src)) {
					case 0: tok = "Gdb1"; break;
					case 1: tok = "Gdb2"; break;
					case 2: tok = "Gdb3"; break;
					case 3: tok = "Gdb4"; break;
					case 4: tok = "Gdb5"; break;
					case 5: tok = "Gdb6"; break;
					case 6: tok = "Gdb7"; break;
					case 7: tok = "Gdb8"; break;
					case 8: tok = "Gdb9"; break;
					case 9: tok = "Gdb0"; break;
					default:tok = "Gdb?"; break;
				}
				break;
			case 0x62:
			case 0x63:
				++src;
				tok = "‹??›";
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