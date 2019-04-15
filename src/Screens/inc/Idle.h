#pragma once

#include "MainEngine.h"

class Idle : public BaseScreen {
public:
	Idle(MainEngine * mainEngine);
	virtual ~Idle();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
private:
	MainEngine * mainEngine;
};

