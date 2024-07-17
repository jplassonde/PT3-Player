#pragma once

#include "ScreenElement.h"
#include <functional>
#include <cstdint>

class Button : public ScreenElement {
public:
	Button(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t img, std::function<void()> cb);
	virtual ~Button();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);

	template<typename T>
	void registerFunction(void (T::*funct)(uint8_t), T * obj) {
		callBack = std::bind(funct, obj, std::placeholders::_1);
	}
private:
	bool pressed;
	uint32_t img;
	std::function<void()> callBack;
};
