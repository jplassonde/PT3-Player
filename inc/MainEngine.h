#pragma once

#include <cstdint>
#include "BaseScreen.h"
#include "Font.h"
#include "Screen.h"
#include "ff.h"

extern Font * rezFont18px;
extern Font * rezFont27px;
class Browser;
class PlayerDisplay;
class Idle;
class Controls;

class MainEngine {
public:
	MainEngine();
	virtual ~MainEngine();
	void run();
	uint32_t getTick();
	uint32_t getColor(uint16_t xPos, uint16_t yPos);
	void switchScreen(BaseScreen * screen);
	void play();
	void browse();
	void showControls();
	void drawBackground();
	Idle * idle;
	Browser * browser;
	PlayerDisplay * pd;
	Controls * controls;
private:
	void addScanlines();
	uint32_t tick;
	BaseScreen * currentScreen;
};
