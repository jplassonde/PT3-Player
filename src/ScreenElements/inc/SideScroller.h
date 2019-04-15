#pragma once

#include "Font.h"
#include <cstdint>
#include "ScreenElement.h"

class SideScroller : public ScreenElement {
public:
	SideScroller(uint16_t xPos, uint16_t yPos, uint16_t _xSize, Font * font, const uint8_t scrollSpeed, const char * str);
	virtual ~SideScroller();
	void draw();
private:
	void drawChar(const uint8_t * const character, uint8_t xStart, uint8_t xStop, uint16_t xPos, uint32_t framebuffer);

	uint32_t getColor(uint16_t xPos, uint16_t yPos);
	uint16_t scrollIndex;
	uint16_t totalWidth;
	uint8_t charWidth;
	Font * font;
	char * text;
	const uint8_t scrollSpeed;
};
