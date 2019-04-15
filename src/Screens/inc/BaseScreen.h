#pragma once

#include "TouchEvent.h"
#include "ScreenElement.h"
#include "Text.h"
#include "Button.h"
#include <memory>
#include <vector>

extern Font * rezFont18px;
extern Font * rezFont27px;

class BaseScreen {
public:
	virtual ~BaseScreen() {}
	virtual void processTouch(TOUCH_EVENT_T touchEvent) = 0;
	virtual void drawScreen() = 0;
protected:
	std::vector<std::unique_ptr<ScreenElement>> screenElemV;
};
