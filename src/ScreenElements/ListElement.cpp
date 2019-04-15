#include "ListElement.h"
#include <cmath>
#include <cstring>
#include "listitem750x50.h"
#include "Font.h"
#include "Printer.h"

extern Font * rezFont27px;

constexpr uint16_t HEIGHT = 50;
constexpr uint16_t WIDTH = 750;
constexpr uint16_t TEXTWIDTH = 700;
constexpr uint16_t FONTSIZE = 27;

ListElement::ListElement(FILINFO * fno, std::function<void(TCHAR *, FSIZE_t)> cb) : callBack(cb){
		name = (TCHAR*)pvPortMalloc((strlen(fno->fname)+1)*sizeof(TCHAR));
		strcpy(name, fno->fname);
		fno->fattrib & AM_DIR ? type = ISDIR : type = ISFILE;
		textStart = 0;
		fileSize = fno->fsize;
}

ListElement::~ListElement() {
		vPortFree(name);
}

void ListElement::draw() {
	constexpr uint8_t topPadding = 12;

	if (yPosition <= 1 || yPosition >= 480) { // Check if is in the display area
		return;
	}

	Printer::IMG_PRINT_T imgCfg = {0};
	imgCfg.imgAddress = (uint32_t)listitem;
	imgCfg.imgWidth = 750;
	imgCfg.topCrop = (yPosition < 50 ? 50-yPosition : 0);
	imgCfg.height = (yPosition > 430 ? 480 - yPosition : 50) - imgCfg.topCrop;
	imgCfg.xPosition = 0;
	imgCfg.yPosition = yPosition + imgCfg.topCrop;
	Printer::printCroppedImg(&imgCfg);

	uint16_t textY = yPosition + topPadding;

	if (textY <= 50-FONTSIZE || textY >= 480) { // Check if text is in the display area
		return;
	}

	Printer::STR_PRINT_T strCfg = {0};
	strCfg.str = name;
	strCfg.font = rezFont27px;
	strCfg.length = 700;
	strCfg.leftCrop = textStart;
	strCfg.topCrop = (textY < 50 ? 50-textY : 0);
	strCfg.height = (textY + FONTSIZE >= 480 ? 480 - textY : FONTSIZE) -  strCfg.topCrop;
	strCfg.xPosition = 20;
	strCfg.yPosition = textY + strCfg.topCrop;

	Printer::printCroppedString(&strCfg);
}

void ListElement::press(TOUCH_EVENT_T touchEvent) {
	isPressed = true;
	lastX = xStart = touchEvent.xPosition;
}

void ListElement::contact(TOUCH_EVENT_T touchEvent) {
	// Adjust text starting position on swipe events.
	if (isPressed) {
		if (touchEvent.gesture == SWIPE_LEFT) {
			int32_t maxOffset = strlen(name)*FONTSIZE - TEXTWIDTH;
			if (maxOffset > 0) {
				textStart = (textStart + touchEvent.gestureMagnitude < maxOffset ? textStart + touchEvent.gestureMagnitude : maxOffset);
			}
		}
		if (touchEvent.gesture == SWIPE_RIGHT) {
			textStart = (textStart - touchEvent.gestureMagnitude > 0 ? textStart - touchEvent.gestureMagnitude : 0);
		}
	}
}

void ListElement::liftOff(TOUCH_EVENT_T touchEvent) {
	if (isPressed && abs(touchEvent.xPosition-xStart) < 25) {
		callBack(name, fileSize);
	}
}

bool ListElement::isInside(uint16_t x, uint16_t y) {
	if (y >= yPosition && y <= yPosition + 50 && x <= WIDTH ) {
		return true;
	}
	isPressed = false;
	return false;
}

void ListElement::setY(int16_t pos) {
	isPressed = false;
	yPosition = pos;
}

TCHAR * ListElement::getName() {
	return name;
}

uint8_t ListElement::getType() {
	return type;
}

