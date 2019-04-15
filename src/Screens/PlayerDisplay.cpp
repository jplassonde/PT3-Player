#include <Screens/inc/PlayerDisplay.h>
#include "project.h"
#include "Font.h"
#include "SideScroller.h"
#include <cstring>

PlayerDisplay::PlayerDisplay(MainEngine * me) : mainEngine(me) {

	const char *  lines[] = {" \xDA\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xBF",
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
							"\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xD9"};

	for (uint8_t i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
		std::unique_ptr<ScreenElement> textLine(new Text(35, (i+1)*27, rezFont27px, lines[i], false));
		screenElemV.push_back(std::move(textLine));
	}
}

PlayerDisplay::~PlayerDisplay() {}

void PlayerDisplay::processTouch(TOUCH_EVENT_T touchEvent) {
	// Nothing. No pause/play/stop/go back to browsing, yet
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

void PlayerDisplay::setInfos(const uint8_t * fn, const uint8_t * mod, const uint8_t * auth) {
	trackInfosV.clear();
	std::unique_ptr<ScreenElement> fileName(new SideScroller(35+2*27, 4*27, 23*27, rezFont27px, 3, (const char *)fn));
	trackInfosV.push_back(std::move(fileName));

	char tmp[33] = {0};
	// Get the last non-space character in module Author/trackname
	remTrailingSpace(tmp, mod);
	std::unique_ptr<ScreenElement> modName(new SideScroller(35+6*27, 8*27, 15*27, rezFont27px, 3, (char *)tmp));
	trackInfosV.push_back(std::move(modName));

	remTrailingSpace(tmp, auth);
	std::unique_ptr<ScreenElement> authName(new SideScroller(35+6*27, 12*27, 15*27, rezFont27px, 3, (char *)tmp));
	trackInfosV.push_back(std::move(authName));

}

static void remTrailingSpace(char * to, const uint8_t * from) {
	for (uint8_t i = 31; i > 0; i--) {
			if (from[i] != ' ') {
				strncpy(to, (const char *)from, i+1);
				to[i+1] = 0;
				return;
			}
		}
}