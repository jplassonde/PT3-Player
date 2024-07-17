#include "project.h"
#include "Button.h"
#include "Printer.h"

Button::Button(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t img, std::function<void()> cb) :
        ScreenElement(x, y, xSize, ySize), img(img), callBack(cb) {
    pressed = false;
}

Button::~Button() {}

void Button::draw() {
    Printer::IMG_PRINT_T imgCfg = { 0 };
    imgCfg.imgAddress = img;
    imgCfg.imgWidth = xSize;
    imgCfg.topCrop = 0;
    imgCfg.height = ySize;
    imgCfg.xPosition = xPos;
    imgCfg.yPosition = yPos;
    Printer::printCroppedImg(&imgCfg);
}

void Button::press(TOUCH_EVENT_T touchEvent) {
    pressed = true;
}

void Button::liftOff(TOUCH_EVENT_T touchEvent) {
    if (pressed) {
        pressed = false;
        callBack();
    }
}

bool Button::isInside(uint16_t x, uint16_t y) {

    if (x >= xPos && x <= xPos + xSize && y >= yPos && y <= yPos + ySize) {
        return true;
    } else {
        pressed = false;
        return false;
    }
}
