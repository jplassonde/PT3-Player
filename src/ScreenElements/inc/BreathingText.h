#pragma once

#include "ScreenElement.h"
#include "Text.h"

class BreathingText: public Text {
public:
	BreathingText(uint16_t x, uint16_t y, Font * font, char * str);
	virtual ~BreathingText();
	void draw();
private:
	uint32_t getColor(uint16_t xPos, uint16_t yPos, uint32_t backCol);
	uint8_t timeIndex;
};

