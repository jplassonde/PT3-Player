#pragma once

#include <AbstractState.h>
#include "mainEngine.h"
#include "ff.h"
#include "ListElement.h"
#include "Scrollbar.h"

class Browser: public AbstractState {
public:
	Browser(MainEngine * mainEngine);
	virtual ~Browser();
	void processTouch(TOUCH_EVENT_T touchEvent);
	void drawScreen();
	void exitButton(uint16_t x, uint16_t y);
	void changeDirectory(TCHAR * name, FSIZE_t size);
private:
	void previousDir(TCHAR * name, FSIZE_t size);
	void play(TCHAR * name, FSIZE_t size);
	bool scrollerCB(float percent);
	std::vector<std::unique_ptr<ListElement>> listV;
	std::unique_ptr<Scrollbar>scrollbar;
	void buildDirList();
	void setItemPos();
	void padScreen(uint8_t padStart);
	void swipeDown(uint16_t mag);
	void swipeUp(uint16_t mag);
	MainEngine * mainEngine;
	FATFS fs;
	FIL fp;
	DIR dp;
	FILINFO fno;
	static TCHAR * path;
	uint16_t offset;
	bool markedForDeletion; // :(
};

