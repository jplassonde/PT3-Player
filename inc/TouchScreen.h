#pragma once

#include "TouchEvent.h"


constexpr uint8_t TS_ADDR 	 	= 0x54;
constexpr uint8_t RXBUFFERSIZE	= 5;

constexpr uint8_t GESTURE_THRESHOLD = 15;
constexpr uint8_t DRIFT_LIM = 10;

class TouchScreen {
public:
	TouchScreen();
	virtual ~TouchScreen();
	void monitorTs();
private:
	void setAddress();
	void getTsData(uint8_t * rxBuffer);
	void processGesture();
	TOUCH_EVENT_T touchData;

	uint16_t lastX;
	uint16_t lastY;
};
