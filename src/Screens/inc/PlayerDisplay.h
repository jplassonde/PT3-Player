#pragma once

#include "BaseScreen.h"
#include "TouchEvent.h"
#include "MainEngine.h"
#include "TrackInfos.h"

class PlayerDisplay : public BaseScreen {
public:
	PlayerDisplay(MainEngine * me);
	virtual ~PlayerDisplay();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
	void setInfos(TrackInfos * ti);
	bool isActive();
private:
	std::vector<std::unique_ptr<ScreenElement>> trackInfosV;
	MainEngine * mainEngine;
};
