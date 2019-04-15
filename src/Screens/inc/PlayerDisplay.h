#pragma once

#include "BaseScreen.h"
#include "TouchEvent.h"
#include "MainEngine.h"

class PlayerDisplay : public BaseScreen {
public:
	PlayerDisplay(MainEngine * me);
	virtual ~PlayerDisplay();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
	void setInfos(const uint8_t * fn, const uint8_t * auth, const uint8_t * mod);
private:
	std::vector<std::unique_ptr<ScreenElement>> trackInfosV;
	MainEngine * mainEngine;
};

// Helper
static void remTrailingSpace(char * to, const uint8_t * from);
