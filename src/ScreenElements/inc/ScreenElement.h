#pragma once

#include "project.h"
#include "TouchEvent.h"

extern SemaphoreHandle_t xDma2dSemaphore;


class ScreenElement {
public:
	ScreenElement();
	ScreenElement(uint16_t x, uint16_t y);
	ScreenElement(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize);
	virtual ~ScreenElement();
	virtual void draw() = 0;
	virtual void press(TOUCH_EVENT_T touchEvent);
	virtual void contact(TOUCH_EVENT_T touchEvent);
	virtual void liftOff(TOUCH_EVENT_T touchEvent);
	virtual void processGesture(uint8_t gesture, uint16_t magnitude);
	virtual bool isInside(uint16_t x, uint16_t y);
	static void (ScreenElement::*eventPtr[3]) (TOUCH_EVENT_T);
protected:
	uint16_t xPos;
	uint16_t yPos;
	uint16_t xSize;
	uint16_t ySize;
};



