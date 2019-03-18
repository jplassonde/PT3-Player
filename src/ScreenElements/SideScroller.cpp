#include <SideScroller.h>
#include "project.h"
#include "string.h"
#include "math.h"
#include "colors.h"
#include "Screen.h"
#include "Printer.h"

SideScroller::SideScroller(uint16_t xPos, uint16_t yPos, uint16_t _xSize,  Font * font, const uint8_t scrollSpeed, const char * str)
															: ScreenElement(xPos,yPos), font(font), scrollSpeed(scrollSpeed) {
	xSize = _xSize;
	ySize = font->getHeight();
	text = (char *)(pvPortMalloc((strlen(str)+4) * sizeof(char)));
	strcpy(text, str);
	strcat(text, " \xF0 ");
	scrollIndex = 0;
	charWidth = font->getWidth();
	totalWidth = charWidth*strlen(text);// + xSize;
}

SideScroller::~SideScroller() {
	vPortFree(text);
}

void SideScroller::draw() {
	STR_PRINT_T ps = {0};

	ps.color = true;
	ps.height = font->getHeight();
	ps.leftCrop = scrollIndex;
	ps.length = xSize;
	ps.xPosition= xPos;
	ps.yPosition = yPos;
	ps.str = text;
	ps.font = font;
	Printer::printCroppedString(&ps);

	// Repeat for whole scroller width
	uint8_t mult = 1;
	while (xSize - (totalWidth*mult-scrollIndex) > 0) {
		ps.length = xSize - (totalWidth*mult-scrollIndex);
		ps.xPosition = xPos+totalWidth*mult-scrollIndex;
		ps.leftCrop = 0;
		Printer::printCroppedString(&ps);
		++mult;
	}

	scrollIndex += scrollSpeed;
	if (scrollIndex >= totalWidth)
		scrollIndex = 0;

	/*
	uint16_t drawPos;
	uint16_t charIndex;
	uint8_t startCrop;
	uint8_t stopCrop = 0;

	if (scrollIndex > xSize) {
		drawPos = xPos;
		charIndex = floor((scrollIndex-xSize)/charWidth);
		startCrop = (scrollIndex-xSize) % charWidth;
	} else {
		drawPos = xPos+xSize-scrollIndex;
		charIndex = 0;
		startCrop = 0;
	}

	if (drawPos + font->getWidth() > xPos+xSize) {
		stopCrop = font->getWidth() - (xPos+xSize-drawPos);
	}


	// Draw first cropped char
	drawChar(font->getChar(text[charIndex]), startCrop, stopCrop, drawPos, Screen::getBackBufferAddr());
	drawPos += charWidth - startCrop;
	++charIndex;

	// Draw others
	while(text[charIndex] && drawPos < xPos+xSize) {
		if (drawPos + charWidth > xPos+xSize)
			stopCrop = charWidth - (xPos+xSize-drawPos);

		drawChar(font->getChar(text[charIndex]), 0, stopCrop, drawPos, Screen::getBackBufferAddr());
		++charIndex;
		drawPos+= charWidth;
	}

	scrollIndex += scrollSpeed;

	if (scrollIndex >= totalWidth+xSize)
		scrollIndex = 1;
		*/
}

void SideScroller::drawCharDma2D(const uint8_t * character, uint8_t xStart, uint8_t xStop, uint16_t xPosition, uint16_t yPosition, uint32_t backbuffer) {
	uint32_t destination = backbuffer + (yPosition * 800 + xPosition) * 4;
	hdma2d.Init.Mode         = DMA2D_M2M_BLEND;
	hdma2d.Init.OutputOffset = 800 - (charWidth - xStart - xStop);
	hdma2d.LayerCfg[0].InputOffset = 800 - (charWidth - xStart - xStop);
	hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A8;
	hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputOffset = xStart+xStop;

	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 0);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);

	HAL_DMA2D_BlendingStart_IT(&hdma2d, (uint32_t)(character+xStart),
									   destination, destination,
									   charWidth-xStart-xStop, font->getHeight());
	xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

void SideScroller::drawChar(const uint8_t * character, uint8_t xStart, uint8_t xStop, uint16_t xPosition, uint32_t framebuffer) {
	uint32_t destination = framebuffer + (yPos * 800 + xPosition) * 4;

	for (int i = 0; i < font->getHeight(); i++) {
		for (int j = 0; j < (charWidth - xStart - xStop); j++) {
			if (character[i*charWidth+xStart+j] == 0xFF) {
				*(uint32_t *)(destination+4*(i*800+j)) = getColor(xPosition+j, yPos+i);
			}
		}
	}
}

uint32_t SideScroller::getColor(uint16_t xPos, uint16_t yPos) {
	return color2[((xPos+2*yPos+scrollIndex*6)/4)%256];
	//return color[(sinetable[xPos/2]+yPos+scrollIndex)%256];
	//return color[sinetable[(((sinetable[xPos]+sinetable[yPos]+scrollIndex)/4+scrollIndex) % 1080)]];
}
