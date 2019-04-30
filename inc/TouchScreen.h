#pragma once

#include "TouchEvent.h"

class TouchScreen {
public:
	TouchScreen();
	virtual ~TouchScreen();
	void monitorTs();
private:
	void setAddress();
	void getTsData(uint8_t * rxBuffer);
	void findGesture(TOUCH_EVENT_T * touchData);

};
