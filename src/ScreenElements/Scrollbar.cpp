#include "Scrollbar.h"
#include "sidebar50x430.h"
#include "SBCursor50x80.h"
#include "Printer.h"

constexpr uint16_t HEIGHT = 430;
constexpr uint8_t CURSORH = 80;

Scrollbar::Scrollbar(uint16_t x, uint16_t y, std::function<bool(float percent)> cb) :
        ScreenElement(x, y), callBack(cb) {
    isPressed = false;
    cursorPos = 0;
}

Scrollbar::~Scrollbar() {}

void Scrollbar::draw() {
    Printer::IMG_PRINT_T imgCfg = { 0 };
    imgCfg.imgAddress = (uint32_t)sidebar50x430;
    imgCfg.imgWidth = 50;
    imgCfg.height = HEIGHT;
    imgCfg.xPosition = xPos;
    imgCfg.yPosition = yPos;
    Printer::printCroppedImg(&imgCfg);

    imgCfg.imgAddress = (uint32_t)SBCursor50x80;
    imgCfg.height = CURSORH;
    imgCfg.yPosition = yPos + cursorPos;
    Printer::printCroppedImg(&imgCfg);
}
void Scrollbar::press(TOUCH_EVENT_T touchEvent) {
    isPressed = true;
    sendPos(touchEvent.yPosition);
}
void Scrollbar::contact(TOUCH_EVENT_T touchEvent) {
    if (isPressed)
        sendPos(touchEvent.yPosition);
}

void Scrollbar::liftOff(TOUCH_EVENT_T touchEvent) {
    isPressed = false;
}

// Send value in % (1-100) of the possible cursor range
void Scrollbar::sendPos(uint16_t touchY) {
    touchY -= yPos; // set base value to 0 (0-430 possible value)

    //set the "work area" between possible cursor position (40-390)
    if (touchY < CURSORH / 2)
        touchY = CURSORH / 2;
    if (touchY > HEIGHT - CURSORH / 2)
        touchY = HEIGHT - CURSORH / 2;

    touchY -= CURSORH / 2;  // (0-350)

    callBack(touchY * 100 / (HEIGHT - CURSORH));
}

void Scrollbar::setCursor(uint16_t y) {
    cursorPos = y;
}

