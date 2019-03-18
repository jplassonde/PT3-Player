#pragma once

#include <ScreenElement.h>
#include "Font.h"

class Text: public ScreenElement {
public:
	Text(uint16_t x, uint16_t y, Font * font, const char * str, bool color);
	virtual ~Text();
	virtual void draw();
	virtual void setText(const char * const str);
	virtual void setPos(uint16_t xPosition, uint16_t yPosition);
protected:
	Font * font;
	char * text;
	bool color;
};

