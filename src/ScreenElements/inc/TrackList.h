#pragma once


#include "ScreenElement.h"
#include "ListElement.h"
#include "Scrollbar.h"
#include  <vector>
#include <memory>
#include "MainEngine.h"
#include "FsFolder.h"

class TrackList: public virtual ScreenElement {
public:
	TrackList(MainEngine * me);
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void contact(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);
	virtual ~TrackList();
private:
	void padScreen(uint8_t padStart);
	void swipeDown(uint16_t mag);
	void swipeUp(uint16_t mag);
	void setItemPos();
	bool scrollerCB(float percent);
	void play(std::shared_ptr<TCHAR>);
	void buildList();
	void changeDirectory(std::shared_ptr<TCHAR>);
	void previousDir(std::shared_ptr<TCHAR>);

	MainEngine * mainEngine;
	std::vector<std::unique_ptr<ListElement>> listV;
	std::unique_ptr<Scrollbar>scrollbar;
	uint16_t offset;
	FsFolder folder;
};
