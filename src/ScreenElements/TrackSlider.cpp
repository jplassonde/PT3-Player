#include "TrackSlider.h"
#include "TrackTime.h"
#include "Printer.h"
#include <cstdio>
#include <cstring>
#include "PlayerQueue.h"
#include "Screen.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern Font * rezFont18px;
extern TrackTime * trackTime;
extern QueueHandle_t xPlayerCmdQueue;

constexpr uint16_t SLIDER_LENGTH = 400;
constexpr uint16_t SLIDER_HEIGHT = 4;

TrackSlider::TrackSlider(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize) :
        ScreenElement(x, y, xSize, ySize), font(rezFont18px), pressed(false) {
}

TrackSlider::~TrackSlider() {}

void TrackSlider::draw() {
    uint32_t total = trackTime->getTotal() / 1000;
    uint32_t current = trackTime->getCurrent() / 1000;
    uint16_t elapsedLength;
    char buffer[16];

    // Draw currentTime
    snprintf(buffer, 16, "%02lu:%02lu", current / 60, current % 60);
    Printer::STR_PRINT_T strCfg = { 0 };
    strCfg.str = buffer;
    strCfg.font = font;
    strCfg.length = strlen(buffer) * font->getWidth();
    strCfg.height = strCfg.font->getHeight();
    strCfg.xPosition = xPos;
    strCfg.yPosition = yPos + ySize / 2 - strCfg.height / 2;
    Printer::printString(&strCfg);

    // Draw totalTime
    snprintf(buffer, 16, "%02lu:%02lu", total / 60, total % 60);
    strCfg.length = strlen(buffer) * font->getWidth();
    strCfg.xPosition = xPos + xSize - strCfg.length;
    Printer::printString(&strCfg);

    // Draw Slider --> TODO redo/improve/etc
    if (total != 0) {
        elapsedLength = current * SLIDER_LENGTH / total;
    } else {
        elapsedLength = 0;
    }
    uint32_t rowPos;
    uint32_t thisVariableShouldntExist = 0;
    for (uint16_t row = yPos + ySize / 2 - SLIDER_HEIGHT / 2; row <= yPos + ySize / 2 + SLIDER_HEIGHT / 2; row++) {
        rowPos = Screen::getBackBufferAddr() + row * 800 * 4;
        thisVariableShouldntExist = 0;

        for (uint16_t col = xPos + xSize / 2 - SLIDER_LENGTH / 2;
                col <= xPos + xSize / 2 + SLIDER_LENGTH / 2; col++) {
            if (thisVariableShouldntExist <= elapsedLength) {
                *(uint32_t *)(rowPos + col * 4) = Printer::getColor(col, row);
            } else {
                *(uint32_t *)(rowPos + col * 4) = 0xFF7F7F7F;
            }
            ++thisVariableShouldntExist;
        }
    }
}

void TrackSlider::press(TOUCH_EVENT_T touchEvent) {
    pressed = true;
}

void TrackSlider::contact(TOUCH_EVENT_T touchEvent) {
    // update position on the fly later? Pressed -> slider show finger pos, unpress show track pos.. etc..
}

void TrackSlider::liftOff(TOUCH_EVENT_T touchEvent) {
    if (pressed) {
        uint16_t sliderXPos = xPos + xSize / 2 - SLIDER_LENGTH / 2;
        uint16_t sliderEndPos = xPos + xSize / 2 + SLIDER_LENGTH / 2;
        uint16_t posInSlider;

        if (touchEvent.xPosition >= sliderEndPos) {
            posInSlider = SLIDER_LENGTH - 1;
        } else if (touchEvent.xPosition <= sliderXPos) {
            posInSlider = 0;
        } else {
            posInSlider = touchEvent.xPosition - sliderXPos;
        }

        uint32_t newTrackPos = trackTime->getTotal() * posInSlider / SLIDER_LENGTH;
        PLAYER_QUEUE_T pq;
        pq.cmd = FF;
        pq.pos = newTrackPos;
        xQueueSend(xPlayerCmdQueue, (void *)&pq, portMAX_DELAY);
    }
}

bool TrackSlider::isInside(uint16_t x, uint16_t y) {
    if (x >= xPos && x <= xPos + xSize && y >= yPos && y <= yPos + ySize) {
        return true;
    } else {
        pressed = false;
        return false;
    }
}
