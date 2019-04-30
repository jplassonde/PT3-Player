#include "ListElement.h"
#include <cmath>
#include <cstring>
#include "listitem750x50.h"
#include "Font.h"
#include "Printer.h"
#include "trackIcon27x27.h"
#include "folderIcon27x27.h"

extern Font * rezFont27px;

constexpr uint16_t HEIGHT = 50;
constexpr uint16_t WIDTH = 750;
constexpr uint16_t TEXTWIDTH = 650;
constexpr uint16_t FONTSIZE = 27;

ListElement::ListElement(std::shared_ptr<TCHAR> dirName, std::function<
        void(std::shared_ptr<TCHAR>)> cb) :
        callBack(cb) {
    name = dirName;
    textStart = 0;
    icon = (uint32_t)folderIcon27x27;
}

ListElement::ListElement(std::shared_ptr<FileInfos> fi, std::function<
        void(std::shared_ptr<TCHAR>)> cb) :
        callBack(cb) {
    name = fi->getName();
    textStart = 0;
    icon = (uint32_t)trackIcon27x27;
}

ListElement::~ListElement() {
}

void ListElement::draw() {
    constexpr uint8_t topPadding = 12;

    if (yPosition <= 1 || yPosition >= 480) { // Check if is in the display area. May not need this check anymore.
        return;
    }

    Printer::IMG_PRINT_T imgCfg = { 0 };
    imgCfg.imgAddress = (uint32_t)listitem;
    imgCfg.imgWidth = 750;
    imgCfg.topCrop = (yPosition < 50 ? 50 - yPosition : 0);
    imgCfg.height = (yPosition > 430 ? 480 - yPosition : 50) - imgCfg.topCrop;
    imgCfg.xPosition = 0;
    imgCfg.yPosition = yPosition + imgCfg.topCrop;
    Printer::printCroppedImg(&imgCfg);

    uint16_t textY = yPosition + topPadding;

    if (textY <= 50 - FONTSIZE || textY >= 480) // Check if text/icon is in the display area
        return;

    imgCfg.imgAddress = icon;
    imgCfg.imgWidth = 27;
    imgCfg.topCrop = (textY < 50 ? 50 - textY : 0);
    imgCfg.height = (textY + FONTSIZE >= 480 ? 480 - textY : FONTSIZE) - imgCfg.topCrop;
    imgCfg.xPosition = 10;
    imgCfg.yPosition = textY + imgCfg.topCrop;
    Printer::printCroppedImg(&imgCfg);

    Printer::STR_PRINT_T strCfg = { 0 };
    strCfg.str = name.get();
    strCfg.font = rezFont27px;
    strCfg.length = TEXTWIDTH;
    strCfg.leftCrop = textStart;
    strCfg.topCrop = imgCfg.topCrop;
    strCfg.height = imgCfg.height;
    strCfg.xPosition = 50;
    strCfg.yPosition = imgCfg.yPosition;

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
            int32_t maxOffset = strlen(name.get()) * FONTSIZE - TEXTWIDTH;
            if (maxOffset > 0) {
                textStart = (
                        textStart + touchEvent.gestureMagnitude < maxOffset ?
                                textStart + touchEvent.gestureMagnitude :
                                maxOffset);
            }
        }
        if (touchEvent.gesture == SWIPE_RIGHT) {
            textStart = (
                    textStart - touchEvent.gestureMagnitude > 0 ?
                            textStart - touchEvent.gestureMagnitude : 0);
        }
    }
}

void ListElement::liftOff(TOUCH_EVENT_T touchEvent) {
    if (isPressed && abs(touchEvent.xPosition - xStart) < 25) {
        callBack(name);
    }
}

bool ListElement::isInside(uint16_t x, uint16_t y) {
    if (y >= yPosition && y <= yPosition + 50 && x <= WIDTH)
        return true;

    isPressed = false;
    return false;
}

void ListElement::setY(int16_t pos) {
    isPressed = false;
    yPosition = pos;
}
