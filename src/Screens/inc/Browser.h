#pragma once

#include <Screens/inc/BaseScreen.h>
#include "MainEngine.h"

class Browser: public BaseScreen {
public:
	Browser(MainEngine * me);
	virtual ~Browser();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
};

