#pragma once
#include "Font.h"

namespace Printer {
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
	} IMG_PRINT_T;

	void printString(STR_PRINT_T * printStruct);
	void printColString(STR_PRINT_T * printStruct);
	void printCroppedString(STR_PRINT_T * printStruct);
	void printImg(IMG_PRINT_T * printStruct);
	void printARGB(IMG_PRINT_T * printStruct);
	void printCroppedImg(IMG_PRINT_T * printStruct);
	void fill(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t color);
	uint32_t getColor(uint16_t x, uint16_t y);
};
