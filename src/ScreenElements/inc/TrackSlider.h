#pragma once

#include <ScreenElement.h>
#include "Font.h"
class TrackSlider: public ScreenElement {
public:
	TrackSlider(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize);
	virtual ~TrackSlider();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void contact(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);
private:
	Font * font;
	bool pressed;
};
