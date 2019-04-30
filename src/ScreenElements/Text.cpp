#include "Text.h"
#include "string.h"
#include <cstdlib>
#include "project.h"
#include "Printer.h"

Text::Text(uint16_t x, uint16_t y, Font * font, const char * str, bool color) :
        ScreenElement(x, y), font(font), color(color) {
    xSize = font->getWidth() * strlen(str);
    ySize = font->getHeight();
    text = (char*)pvPortMalloc((strlen(str) + 1) * sizeof(char));
    strcpy(text, str);
}

Text::~Text() {
    vPortFree(text);
}

void Text::draw() {
    Printer::STR_PRINT_T printCfg = { 0 };
    printCfg.xPosition = xPos;
    printCfg.yPosition = yPos;
    printCfg.font = font;
    printCfg.str = text;
    if (color) {
        Printer::printColString(&printCfg);
    } else {
        Printer::printString(&printCfg);
    }
}

void Text::setText(const char * const str) {
    vPortFree(text);
    text = (char*)pvPortMalloc((strlen(str) + 1) * sizeof(char));
    strcpy(text, str);
}

void Text::setPos(uint16_t xPosition, uint16_t yPosition) {
    xPos = xPosition;
    yPos = yPosition;
}
