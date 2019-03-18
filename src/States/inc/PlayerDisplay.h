#pragma once

#include "TouchEvent.h"
#include "AbstractState.h"
#include "MainEngine.h"

class PlayerDisplay : public AbstractState {
public:
	PlayerDisplay(const uint8_t * fn, const uint8_t * mod, const uint8_t * auth, MainEngine * me);
	virtual ~PlayerDisplay();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
private:
	const uint8_t * filename;
	const uint8_t * modName;
	const uint8_t * author;
	MainEngine * mainEngine;
};
