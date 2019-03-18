#pragma once

#include "ScreenElement.h"
#include <functional>
#include <cstdint>

class Button : public ScreenElement {
public:
	Button(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t img, uint32_t imgPressed, std::function<void(uint16_t, uint16_t)> cb);
	virtual ~Button();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void contact(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);
private:
	bool pressed;
	uint32_t img;
	uint32_t pressedImg;
	std::function<void(uint16_t,uint16_t)> callBack;
};
