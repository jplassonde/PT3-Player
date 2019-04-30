#pragma once

#include "ScreenElement.h"
#include <functional>
class Scrollbar: public ScreenElement {
public:
	Scrollbar(uint16_t x, uint16_t y, std::function<bool(float percent)> cb);
	virtual ~Scrollbar();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void contact(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	void setCursor(uint16_t y);
private:
	void sendPos(uint16_t touchY);
	std::function<bool(float percent)> callBack;
	bool isPressed;
	uint16_t cursorPos;
};

