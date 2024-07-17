#pragma once
#include "ScreenElement.h"
#include <Functional>

typedef struct VerticalSlider_t {
	uint16_t x;
	uint16_t y;
	uint16_t xSize;
	uint16_t ySize;
} VerticalSlider_t;

class VerticalSlider : public ScreenElement {

public:
	VerticalSlider(VerticalSlider_t * init);
	virtual ~VerticalSlider();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);

	template<typename T>
	void registerFunction(void (T::*funct)(uint8_t), T * obj) {
		update = std::bind(funct, obj, std::placeholders::_1);
	}
	void registerFunction(void (*funct)(uint8_t));

private:
	bool pressed;
	uint16_t slidePos;
	std::function<void(uint8_t)> update;
};

