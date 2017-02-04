#include "tokens.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

static char const * tok_tbl[256] = 
{
	"NUL", // 0
	"▸DMS", // 1
	"▸DEC", // 2
	"▸FRAC", // 3
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
	"▸5C‹", // 92
	"▸5D‹", // 93
	"▸5E‹", // 94
	"prgm", // 95
	"▸60‹", // 96
	"▸61‹", // 97
	"▸62‹", // 98
	"▸63‹", // 99
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
	"▸7E‹", // 126
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
	"▸AA‹", // 170
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
	"▸BB‹",
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


char const * tok_tbl0x5C[] = {
	"[A]", "[B]", "[C]", "[D]", "[E]", "[F]", "[G]", "[H]", "[I]", "[J]" 
};
char const * tok_tbl0x5D[] = {
	"L₁", "L₂", "L₃", "L₄", "L₅", "L₆", "L₇", "L₈", "L₉", "L₀"
};
char const * tok_tbl0x5E[] = {
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"Y₁", "Y₂", "Y₃", "Y₄", "Y₅", "Y₆", "Y₇", "Y₈", "Y₉", "Y₀", "·", "·", "·", "·", "·", "·",
	"X₁ₜ", "Y₁ₜ", "X₂ₜ", "Y₂ₜ", "X₃ₜ", "Y₃ₜ", "X₄ₜ", "Y₄ₜ", "X₅ₜ", "Y₅ₜ", "X₆ₜ", "Y₆ₜ", "·","·","·","·",
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"r₁", "r₂", "r₃", "r₄", "r₅", "r₆", "·","·","·","·","·","·","·","·","·","·",
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·", "·",
	"u", "v",
};
char const * tok_tbl0x60[] = {
	"Pic0", "Pic1", "Pic2", "Pic3", "Pic4", "Pic5", "Pic6", "Pic7", "Pic8", "Pic9", "Pic0"
};
char const * tok_tbl0x61[] = {
	"GDB0", "GDB1", "GDB2", "GDB3", "GDB4", "GDB5", "GDB6", "GDB7", "GDB8", "GDB9", "GDB0"
};
char const * tok_tbl0x62[] = {
	"·", "RegEq", "n", "x̅", "Σx", "Σx²", "Sx", "σx", "minX", "maxX", "minY", "maxY", "y̅", "Σy", "Σy²", "Sy", "σy", "Σxy", "r", "Med", "Q₁", "Q₃", "a", "b", "c", "d", "e", "x₁", "x₂", "x₃", "y₁", "y₂", "y₃", "𝑛", "p", "z", "t", "χ²", "𝔽", "df", "p̂", "p̂₁", "p̂₂", "x̅₁", "Sx₁", "n₁", "x̅₂", "Sx₂", "n₂", "Sxp", "lower", "upper", "s", "r²", "R²", "df", "SS", "MS", "df", "SS", "MS"
};
char const * tok_tbl0x63[] = {
	"ZXscl", "ZYscl", "Xscl", "Yscl", "U𝑛Start", "V𝑛Start", "U𝑛₋₁", "V𝑛₋₁", "ZU𝑛Start", "ZV𝑛Start", "Xmin", "Xmax", "Ymin", "Ymax", "Tmin", "Tmax", "θmin", "θmax", "ZXMin", "ZXmax", "ZYmin", "ZYmax", "Zθmin", "Zθmax", "ZTmin", "ZTmax", "TblMin", "𝑛Min", "Z𝑛Min", "𝑛Max", "Z𝑛max", "𝑛Start", "Z𝑛Start", "ΔTbl", "Tstep", "θstep", "ZTstep", "Zθstep", "ΔX", "ΔY", "XFact", "YFact", "TblInput", "ℕ", "I%", "PV", "PMT", "FV", "Xres", "ZXres"
};
char const * tok_tbl0xAA[] = {
	"Str0", "Str1", "Str2", "Str3", "Str4", "Str5", "Str6", "Str7", "Str8", "Str9", "Str0"
};
char const * tok_tbl0x7E[] = {
	"Sequential", "Simul", "PolarGC", "RectGC", "CoordOn", "CoordOff", "Connected", "Dot", "AxesOn", "AxesOff", "GridOn", "GridOff",
	"uvAxes", "vwAxes", "uwAxes",
};
char const * tok_tbl0xBB[] = {
	"npv(", "irr(", "bal(", "ΣPrn(", "ΣInt(", "▸Nom(", "▸Eff(", "dbd(", "lcm(", "gcd(", "randInt(", "randBin(", "sub(", "stdDev(", "variance(", "inString(", 
	"normalcdf(", "invNorm(", "tcdf(", "χ²cdf(", "𝔽cdf(", "binompdf(", "binomcdf(", "poissonpdf(", "poissoncdf(", "geometpdf(", "geometcdf(", "normalpdf(", "tpdf(", "χ²pdf(", "𝔽pdf(", "randNorm(",
	"tvm_pmt", "tvm_I%", "tvm_PV", "tvm_ℕ", "tvm_FV", "conj(", "real(", "imag(", "angle(", "cumSum(", "expr(", "length(", "ΔList(", "ref(", "rref(", "▸Rect", 
	"▸Polar", "e", "SinReg ", "Logistic ", "LinRegTTest ", "ShadeNorm(", "Shade_t(", "Shadeχ²(", "Shade𝔽(", "Matr▸list", "List▸matr", "Z-Test(", "T-Test ", "2-SampZTest(", "1-PropZTest(", "2-PropZTest(", 
	"χ²-Test(", "ZInterval ", "2-SampZInt(", "1-PropZInt(", "2-PropZInt(", "GraphStyle(", "2-SampTTest ", "2-Samp𝔽Test ", "TInterval ", "2-SampTInt ", "SetUpEditor ", "PMT_End", "PMT_Bgn", "Real", "re^θi", "a+bi", 
	"ExprOn", "ExprOff", "ClrAllLists", "GetCalc(", "DelVar ", "Equ▸String(", "String>Equ(", "Clear Entries", "Select(", "ANOVA(", "ModBoxPlot", "NormProbPlot"
};

char const * tok_getFromTable(uint8_t index, char const ** table, size_t length)
{
	if(index >= length) {
		error_message("Invalid token detected!\n");
		exit(EXIT_FAILURE);
	}
	return table[index];
}

char const * tok_getTwoByte(uint8_t first, uint8_t second)
{
#define GET(X) case X: return tok_getFromTable(second, tok_tbl##X, sizeof(tok_tbl##X) / sizeof(tok_tbl##X[0]))
	switch(first) {
		GET(0x5C);
		GET(0x5D);
		GET(0x5E);
		GET(0x60);
		GET(0x61);
		GET(0x62);
		GET(0x63);
		GET(0x7E);
		GET(0xAA);
		GET(0xBB);
		default:
			error_message("Invalid token detected: %02X%02X\n", (int)first, (int)second);
			exit(EXIT_FAILURE);
			return "??";
	}
}

char const * tok_getOneByte(uint8_t first)
{
	return tok_tbl[first];
}

bool tok_isTwoByte(uint8_t tok)
{
	switch(tok) {
		case 0x5C: return true;
		case 0x5D: return true;
		case 0x5E: return true;
		case 0x60: return true;
		case 0x61: return true;
		case 0x62: return true;
		case 0x63: return true;
		case 0x7E: return true;
		case 0xAA: return true;
		case 0xBB: return true;
		default: return false;
	}
}

void detokenize(char * dst, uint8_t const * src, int len)
{
	for(int i = 0; i < len; i++)
	{
		char const * tok = NULL;
		uint8_t tok1 = *src;
		
		if(tok1 == 0) {
			break;
		}
		
		if(tok_isTwoByte(tok1)) {
			uint8_t tok2 = *(++src);
			tok = tok_getTwoByte(tok1, tok2);
		} else {
			tok = tok_getOneByte(tok1);
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