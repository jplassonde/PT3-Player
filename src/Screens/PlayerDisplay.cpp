#include <Screens/inc/PlayerDisplay.h>
#include "project.h"
#include "Font.h"
#include "SideScroller.h"
#include <cstring>
#include "Button.h"
#include "listtab200x50.h"
#include "playtab200x50.h"
#include "TrackSlider.h"
#include "usbh_g935.h"
#include "Wm8994.h"

PlayerDisplay::PlayerDisplay(MainEngine * me) :
        mainEngine(me) {

    const char * lines[] =
            {
                    " \xDA\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xBF",
                    " \xB3        < NFO >        \xB3",
                    " \xC3\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4 File: \xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xB4",
                    " \xB3                       \xB3",
                    " \xC0\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2\xD9",
                    "\xC4\xC4\xB4 \x0E  -=\xF0 Title \xF0=-  \x0E \xC3\xC4\xC4",
                    "\xDA\xC4\xC1\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC1\xC4\xBF",
                    "\xB3\x0E \xF0=-               -=\xF0 \x0E\xB3",
                    "\xC0\xC4\xC2\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC2\xC4\xD9",
                    "\xC4\xC4\xB4 \x02 -=\xF0 Made by \xF0=- \x02 \xC3\xC4\xC4",
                    "\xDA\xC4\xC1\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC1\xC4\xBF",
                    "\xB3\x02 \xF0=-               -=\xF0 \x02\xB3",
                    "\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9"
            };

    for (uint8_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++) {
        std::unique_ptr<ScreenElement> textLine(new Text(35, (i + 2) * 27, rezFont27px, lines[i], false));
        screenElemV.push_back(std::move(textLine));
    }

    std::unique_ptr<ScreenElement> slider(
            new TrackSlider(100, 400, 600, 50));
    screenElemV.push_back(std::move(slider));

    auto controlButton = std::bind(&MainEngine::showControls, mainEngine);
       std::unique_ptr<ScreenElement> controltab(new Button(200, 0, 200, 50, (uint32_t)playtab200x50, controlButton));
       screenElemV.push_back(std::move(controltab));

    auto browseCallback = std::bind(&MainEngine::browse, mainEngine);
    std::unique_ptr<ScreenElement> browseTab(
            new Button(0, 0, 200, 50, (uint32_t)listtab200x50, browseCallback));
    screenElemV.push_back(std::move(browseTab));
}

PlayerDisplay::~PlayerDisplay() {
}

void PlayerDisplay::processTouch(TOUCH_EVENT_T touchEvent) {
    for (auto &i : screenElemV) {
        if (i->isInside(touchEvent.xPosition, touchEvent.yPosition)) {
            (*i.*(ScreenElement::eventPtr[touchEvent.touchEvent]))(touchEvent);
        }
    }
}

void PlayerDisplay::drawScreen() {
    mainEngine->drawBackground();
    for (auto &i : screenElemV) {
        i->draw();
    }
    for (auto &i : trackInfosV) {
        i->draw();
    }
}

void PlayerDisplay::setInfos(TrackInfos * ti) {
    trackInfosV.clear();

    std::unique_ptr<ScreenElement> fileName(
            new SideScroller(35 + 2 * 27, 5 * 27, 23 * 27, rezFont27px, 3, ti->getFilename()));
    trackInfosV.push_back(std::move(fileName));

    std::unique_ptr<ScreenElement> modName(
            new SideScroller(35 + 6 * 27, 9 * 27, 15 * 27, rezFont27px, 3, ti->getTrackName()));
    trackInfosV.push_back(std::move(modName));

    std::unique_ptr<ScreenElement> authName(
            new SideScroller(35 + 6 * 27, 13 * 27, 15 * 27, rezFont27px, 3, ti->getAuthor()));
    trackInfosV.push_back(std::move(authName));

    delete ti;
}

// Return the status of the player, ie: don't allow to switch to the player screen if no
// track is playing

bool PlayerDisplay::isActive() {
    return trackInfosV.size();
}
