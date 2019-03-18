#pragma once

#include <cstdint>
#include "AbstractState.h"
#include "Font.h"
#include "Screen.h"

extern Font * rezFont18px;
extern Font * rezFont27px;

class MainEngine {
public:
	MainEngine();
	virtual ~MainEngine();
	void run();
	uint32_t getTick();
	uint32_t getColor(uint16_t xPos, uint16_t yPos);
	void switchState(AbstractState * state);
	void drawBackground();
	void addScanlines();
private:
	uint32_t tick;
	AbstractState * currentState;
	Screen * screen;
};
