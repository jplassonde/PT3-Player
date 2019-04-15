#include <rezFont18px.h>
#include <rezFont27px.h>
#include "Font.h"

Font * rezFont18px;
Font * rezFont27px;

void fontInit() {
	rezFont18px = new Font(Font::rez18px);
	rezFont27px = new Font(Font::rez27px);
}

Font::Font(fontSelect fontSel) {
	switch (fontSel) {
	case rez18px:
		fontMap = new std::map<char, const uint8_t *>(
		{
			{240, char240_18px}, {1, char1_18px}, {2, char2_18px}, {3, char3_18px}, {192, char192_18px}, {193, char193_18px}, {194, char194_18px},
			{248, char248_18px}, {179, char179_18px}, {180, char180_18px}, {195, char195_18px}, {196, char196_18px}, {197, char197_18px},
			{228, char228_18px}, {14, char14_18px}, {191, char191_18px}, {32, char32_18px}, {33, char33_18px}, {34, char34_18px},
			{35, char35_18px}, {36, char36_18px}, {37, char37_18px}, {38, char38_18px}, {39, char39_18px}, {40, char40_18px}, {41, char41_18px},
			{42, char42_18px}, {43, char43_18px}, {44, char44_18px}, {45, char45_18px}, {46, char46_18px}, {47, char47_18px}, {48, char48_18px},
			{49, char49_18px}, {50, char50_18px}, {51, char51_18px}, {52, char52_18px}, {53, char53_18px}, {54, char54_18px}, {55, char55_18px},
			{56, char56_18px}, {57, char57_18px}, {58, char58_18px}, {59, char59_18px}, {60, char60_18px}, {61, char61_18px}, {62, char62_18px},
			{63, char63_18px}, {64, char64_18px}, {65, char65_18px}, {66, char66_18px}, {67, char67_18px}, {68, char68_18px}, {69, char69_18px},
			{70, char70_18px}, {71, char71_18px}, {72, char72_18px}, {73, char73_18px}, {74, char74_18px}, {75, char75_18px}, {76, char76_18px},
			{77, char77_18px}, {78, char78_18px}, {79, char79_18px}, {80, char80_18px}, {81, char81_18px}, {82, char82_18px}, {83, char83_18px},
			{84, char84_18px}, {85, char85_18px}, {86, char86_18px}, {87, char87_18px}, {88, char88_18px}, {89, char89_18px}, {90, char90_18px},
			{91, char91_18px}, {92, char92_18px}, {93, char93_18px}, {94, char94_18px}, {95, char95_18px}, {96, char96_18px}, {97, char97_18px},
			{98, char98_18px}, {99, char99_18px}, {100, char100_18px}, {101, char101_18px}, {102, char102_18px}, {103, char103_18px},
			{104, char104_18px}, {105, char105_18px}, {106, char106_18px}, {107, char107_18px}, {108, char108_18px}, {109, char109_18px},
			{110, char110_18px}, {111, char111_18px}, {112, char112_18px}, {113, char113_18px}, {114, char114_18px}, {115, char115_18px},
			{116, char116_18px}, {117, char117_18px}, {118, char118_18px}, {119, char119_18px}, {120, char120_18px}, {121, char121_18px},
			{122, char122_18px}, {217, char217_18px}, {218, char218_18px}, {219, char219_18px}, {220, char220_18px}, {221, char221_18px}
		});
		height = 18;
		width = 18;
		break;
	case rez27px:
		fontMap = new std::map<char, const uint8_t *>(
		{
			{240, char240_27px}, {1, char1_27px}, {2, char2_27px}, {3, char3_27px}, {192, char192_27px}, {193, char193_27px}, {194, char194_27px}, {248, char248_27px},
			{179, char179_27px}, {180, char180_27px}, {195, char195_27px}, {196, char196_27px}, {197, char197_27px}, {228, char228_27px}, {14, char14_27px},
			{191, char191_27px}, {32, char32_27px}, {33, char33_27px}, {34, char34_27px}, {35, char35_27px}, {36, char36_27px}, {37, char37_27px}, {38, char38_27px},
			{39, char39_27px}, {40, char40_27px}, {41, char41_27px}, {42, char42_27px}, {43, char43_27px}, {44, char44_27px}, {45, char45_27px}, {46, char46_27px},
			{47, char47_27px}, {48, char48_27px}, {49, char49_27px}, {50, char50_27px}, {51, char51_27px}, {52, char52_27px}, {53, char53_27px}, {54, char54_27px},
			{55, char55_27px}, {56, char56_27px}, {57, char57_27px}, {58, char58_27px}, {59, char59_27px}, {60, char60_27px}, {61, char61_27px}, {62, char62_27px},
			{63, char63_27px}, {64, char64_27px}, {65, char65_27px}, {66, char66_27px}, {67, char67_27px}, {68, char68_27px}, {69, char69_27px}, {70, char70_27px},
			{71, char71_27px}, {72, char72_27px}, {73, char73_27px}, {74, char74_27px}, {75, char75_27px}, {76, char76_27px}, {77, char77_27px}, {78, char78_27px},
			{79, char79_27px}, {80, char80_27px}, {81, char81_27px}, {82, char82_27px}, {83, char83_27px}, {84, char84_27px}, {85, char85_27px}, {86, char86_27px},
			{87, char87_27px}, {88, char88_27px}, {89, char89_27px}, {90, char90_27px}, {91, char91_27px}, {92, char92_27px}, {93, char93_27px}, {94, char94_27px},
			{95, char95_27px}, {96, char96_27px}, {97, char97_27px}, {98, char98_27px}, {99, char99_27px}, {100, char100_27px}, {101, char101_27px}, {102, char102_27px},
			{103, char103_27px}, {104, char104_27px}, {105, char105_27px}, {106, char106_27px}, {107, char107_27px}, {108, char108_27px}, {109, char109_27px},
			{110, char110_27px}, {111, char111_27px}, {112, char112_27px}, {113, char113_27px}, {114, char114_27px}, {115, char115_27px}, {116, char116_27px},
			{117, char117_27px}, {118, char118_27px}, {119, char119_27px}, {120, char120_27px}, {121, char121_27px}, {122, char122_27px}, {217, char217_27px},
			{218, char218_27px}, {219, char219_27px}, {220, char220_27px}, {221, char221_27px}
		});
		height = 27;
		width = 27;
		break;
	}
	defaultChar = (*fontMap)[219];
}

Font::~Font() {
	delete fontMap;
}

const uint8_t * Font::getChar(uint8_t c) {
	std::map<char,const uint8_t *>::iterator it = fontMap->find(c);
	if (it == fontMap->end()) {
		return defaultChar;
	} else {
		return it->second;
	}
}

uint8_t Font::getWidth() const {
	return width;
}

uint8_t Font::getHeight() const {
	return height;
}
