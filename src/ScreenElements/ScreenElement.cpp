#include "ScreenElement.h"

ScreenElement::ScreenElement() {
}

ScreenElement::ScreenElement(uint16_t x, uint16_t y) :
        xPos(x), yPos(y) {
}

ScreenElement::ScreenElement(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize) :
        xPos(x), yPos(y), xSize(xSize), ySize(ySize) {
}

ScreenElement::~ScreenElement() {
}

void ScreenElement::press(TOUCH_EVENT_T touchEvent) {

}

void ScreenElement::contact(TOUCH_EVENT_T touchEvent) {

}

void ScreenElement::liftOff(TOUCH_EVENT_T touchEvent) {

}

void ScreenElement::processGesture(__attribute__((unused))  uint8_t gesture, __attribute__((unused))  uint16_t magnitude) {

}

bool ScreenElement::isInside(uint16_t x, uint16_t y) {
    if (x >= xPos && x <= xPos + xSize && y >= yPos && y <= yPos + ySize) {
        return true;
    } else {
        return false;
    }
}

void (ScreenElement::*ScreenElement::eventPtr[])(TOUCH_EVENT_T) = {&ScreenElement::press, &ScreenElement::liftOff, &ScreenElement::contact};
