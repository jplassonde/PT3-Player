#include "Browser.h"
#include "Idle.h"
#include "project.h"
#include "Printer.h"
#include <cstring>
#include "BreathingText.h"

//test
#include "SideScroller.h"

Idle::Idle(MainEngine * mainEngine) :
        mainEngine(mainEngine) {

    std::unique_ptr<ScreenElement> insertCoin(new BreathingText(251, 200, rezFont27px, (char *)"INSERT COIN"));
    screenElemV.push_back(std::move(insertCoin));
    std::unique_ptr<ScreenElement> toPlay(new BreathingText(305, 250, rezFont27px, (char *)"TO PLAY"));
    screenElemV.push_back(std::move(toPlay));
}

Idle::~Idle() {}

void Idle::processTouch(TOUCH_EVENT_T touchData) {
    if (touchData.touchEvent == EVENT_UP) {
        mainEngine->switchScreen(mainEngine->browser);
    }
}

void Idle::drawScreen() {
    mainEngine->drawBackground();
    for (auto &i : screenElemV) {
        i->draw();
    }
}
