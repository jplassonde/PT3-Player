#include "Controls.h"
#include "project.h"
#include "Font.h"
#include "playtab200x50.h"
#include "listtab200x50.h"
#include "VerticalSlider.h"
#include "Wm8994.h"
#include <functional>


Controls::Controls(MainEngine *mainEngine) :
			mainEngine(mainEngine){

    auto playButton = std::bind(&MainEngine::play, mainEngine);
    std::unique_ptr<ScreenElement> playtab(new Button(0, 0, 200, 50, (uint32_t)playtab200x50, playButton));
    screenElemV.push_back(std::move(playtab));

    auto browseCallback = std::bind(&MainEngine::browse, mainEngine);
    std::unique_ptr<ScreenElement> browseTab(
            new Button(200, 0, 200, 50, (uint32_t)listtab200x50, browseCallback));
    screenElemV.push_back(std::move(browseTab));

    VerticalSlider_t volInit;
    volInit.x = 100;
    volInit.y = 75;
    volInit.xSize = 80;
    volInit.ySize = 300;

    std::unique_ptr<ScreenElement> vol(new VerticalSlider(&volInit));
    (static_cast<VerticalSlider*>(vol.get()))->registerFunction(&Wm8994::setVolume, Wm8994::getInstance());
    screenElemV.push_back(std::move(vol));

    std::unique_ptr<ScreenElement> volText(new Text(110, 415, rezFont27px, "VOL", false));
    screenElemV.push_back(std::move(volText));
}

Controls::~Controls() {
}

void Controls::processTouch(TOUCH_EVENT_T touchData) {
    for (auto &i : screenElemV) {
        if (i->isInside(touchData.xPosition, touchData.yPosition)) {
            (*i.*(ScreenElement::eventPtr[touchData.touchEvent]))(touchData);
            break;
        }
    }
}

void Controls::drawScreen() {
    mainEngine->drawBackground();
    for (auto &i : screenElemV) {
        i->draw();
    }
}
