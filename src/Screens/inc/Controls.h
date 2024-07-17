#pragma once
#include "BaseScreen.h"
#include "MainEngine.h"

class Controls: public BaseScreen {
public:
	Controls(MainEngine *me);
	virtual ~Controls();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
private:
	MainEngine * mainEngine;
};

