#include "Browser.h"
#include "project.h"
#include "PlayerQueue.h"
#include "TrackList.h"
#include "playtab200x50.h"
#include "listtab200x50.h"
#include <functional>

Browser::Browser(MainEngine * mainEngine) {
    std::unique_ptr<ScreenElement> trackList(new TrackList(mainEngine));
    screenElemV.push_back(std::move(trackList));

    auto controlButton = std::bind(&MainEngine::showControls, mainEngine);
    std::unique_ptr<ScreenElement> controltab(new Button(200, 0, 200, 50, (uint32_t)listtab200x50, controlButton));
    screenElemV.push_back(std::move(controltab));

    auto playCB = std::bind(&MainEngine::play, mainEngine);
    std::unique_ptr<ScreenElement> playtab(new Button(0, 0, 200, 50, (uint32_t)playtab200x50, playCB));
    screenElemV.push_back(std::move(playtab));
}

Browser::~Browser() {
}

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
