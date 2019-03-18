#pragma once
#include "Font.h"

typedef struct STR_PRINT_T {
	char * str;
	uint16_t xPosition;
	uint16_t leftCrop;
	uint16_t length;
	uint16_t yPosition;
	uint16_t topCrop;
	uint16_t height;
	Font * font;
	bool color;
} STR_PRINT_T;

typedef struct IMG_PRINT_T {
	uint32_t imgAddress;
	uint16_t xPosition;
	uint16_t yPosition;
	uint16_t topCrop;
	uint16_t leftCrop;
	uint16_t rightCrop;
	uint16_t imgWidth;
	uint16_t height;
}IMG_PRINT_T;

class Printer {
public:
	static void printString(STR_PRINT_T * printStruct);
	static void printColString(STR_PRINT_T * printStruct);
	static void printCroppedString(STR_PRINT_T * printStruct);
	static void printImg(IMG_PRINT_T * printStruct);
	static void printCroppedImg(IMG_PRINT_T * printStruct);
private:
	static void prvPrintCroppedChar(uint16_t offset, char c, uint8_t leftCrop, uint8_t rightCrop, STR_PRINT_T * printStruct);
	static void prvPrintColChar(uint16_t offset, char c, uint8_t leftCrop, uint8_t rightCrop, STR_PRINT_T * printStruct);
	static uint32_t getColor(uint16_t x, uint16_t y);
	Printer() {}
};
