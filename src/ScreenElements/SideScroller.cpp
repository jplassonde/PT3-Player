#include <SideScroller.h>
#include "project.h"
#include "string.h"
#include "math.h"
#include "colors.h"
#include "Screen.h"
#include "Printer.h"

SideScroller::SideScroller(uint16_t xPos, uint16_t yPos, uint16_t _xSize, Font * font, const uint8_t scrollSpeed, const char * str) :
        ScreenElement(xPos, yPos), font(font), scrollSpeed(scrollSpeed) {
    xSize = _xSize;
    ySize = font->getHeight();
    text = (char *)(pvPortMalloc((strlen(str) + 4) * sizeof(char)));
    strcpy(text, str);
    strcat(text, " \xF0 ");
    scrollIndex = 0;
    charWidth = font->getWidth();
    totalWidth = charWidth * strlen(text); // + xSize;
}

SideScroller::~SideScroller() {
    vPortFree(text);
}

void SideScroller::draw() {
    Printer::STR_PRINT_T ps = { 0 };

    ps.color = true;
    ps.height = font->getHeight();
    ps.leftCrop = scrollIndex;
    ps.length = xSize;
    ps.xPosition = xPos;
    ps.yPosition = yPos;
    ps.str = text;
    ps.font = font;
    Printer::printCroppedString(&ps);

    // Repeat for whole scroller width
    uint8_t mult = 1;
    while (xSize - (totalWidth * mult - scrollIndex) > 0) {
        ps.length = xSize - (totalWidth * mult - scrollIndex);
        ps.xPosition = xPos + totalWidth * mult - scrollIndex;
        ps.leftCrop = 0;
        Printer::printCroppedString(&ps);
        ++mult;
    }

    scrollIndex += scrollSpeed;
    if (scrollIndex >= totalWidth)
        scrollIndex = 0;
}
