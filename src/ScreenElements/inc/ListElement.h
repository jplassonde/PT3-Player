#pragma once

#include <ScreenElement.h>
#include "ff.h"
#include <functional>
#include <memory>
#include "FileInfos.h"

constexpr uint8_t ISDIR = 0;
constexpr uint8_t ISFILE= 1;
constexpr uint8_t ISPAD = 2;

class ListElement: public ScreenElement {
public:
	ListElement(std::shared_ptr<TCHAR> dirName, std::function<void(std::shared_ptr<TCHAR>)> cb);
	ListElement(std::shared_ptr<FileInfos> fi, std::function<void(std::shared_ptr<TCHAR>)> cb);
	virtual ~ListElement();
	void draw();
	void press(TOUCH_EVENT_T touchEvent);
	void contact(TOUCH_EVENT_T touchEvent);
	void liftOff(TOUCH_EVENT_T touchEvent);
	bool isInside(uint16_t x, uint16_t y);
	void setY(int16_t pos);

	TCHAR * getName();
	uint8_t getType();

private:
	FSIZE_t fileSize;
	std::function<void(std::shared_ptr<TCHAR>)> callBack;
	std::shared_ptr<TCHAR> name;
	uint8_t type;
	uint16_t textStart; // Number of pixels to "skip"
	uint16_t lastX;
	uint16_t xStart;
	int32_t xPosition;
	int32_t yPosition;
	bool isPressed;
	uint32_t icon;


};
