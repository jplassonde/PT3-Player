#include "Browser.h"
#include "project.h"
#include "PlayerQueue.h"
#include "TrackList.h"

Browser::Browser(MainEngine * mainEngine) {
	std::unique_ptr<ScreenElement> trackList(new TrackList(mainEngine));
	screenElemV.push_back(std::move(trackList));
}

Browser::~Browser() {}

void Browser::processTouch(TOUCH_EVENT_T touchData) {
	for (auto &i : screenElemV) {
		if (i->isInside(touchData.xPosition, touchData.yPosition)) {
			(*i.*(ScreenElement::eventPtr[touchData.touchEvent]))(touchData);
			break;
		}
	}
}

void Browser::drawScreen() {
	for (auto &i : screenElemV) {
		i->draw();
	}
}
