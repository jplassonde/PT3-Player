#include "BreathingText.h"
#include "Screen.h"
#include "Font.h"
#include "sinetable.h"

BreathingText::BreathingText(uint16_t x, uint16_t y, Font * font, char * str) :
        Text(x, y, font, str, 0) {
}

BreathingText::~BreathingText() {
}

void BreathingText::draw() {
    uint32_t destination = Screen::getBackBufferAddr()
            + (yPos * 800 + xPos) * 4;
    uint8_t strIndex = 0;
    const uint8_t * character;
    while (text[strIndex]) {
        character = font->getChar(text[strIndex]);
        for (int i = 0; i < font->getHeight(); i++) {
            for (int j = 0; j < font->getWidth(); j++) {
                if (character[i * font->getWidth() + j] == 0xFF) {
                    *(uint32_t *)(destination + 4 * (i * 800 + j)) = getColor(xPos + j, yPos + i,
                            *(uint32_t *)(destination + 4 * (i * 800 + j)));
                }
            }
        }
        ++strIndex;
        destination += font->getWidth() * 4;
    }
    timeIndex += 3;
}

// Meh. just give it an uniform color for now. Redo with Printer:: later on
uint32_t BreathingText::getColor(uint16_t xPosition, uint16_t yPosition, uint32_t backCol) {
    uint32_t baseCol = 0xFF007FFF;

    uint8_t r = (((baseCol >> 16) & 0xFF) * sinetable[timeIndex]
            + ((backCol >> 16) & 0xFF) * (255 - sinetable[timeIndex])) / 255;

    uint8_t g = (((baseCol >> 8) & 0xFF) * sinetable[timeIndex]
            + ((backCol >> 8) & 0xFF) * (255 - sinetable[timeIndex])) / 255;

    uint8_t b = ((baseCol & 0xFF) * sinetable[timeIndex]
            + (backCol & 0xFF) * (255 - sinetable[timeIndex])) / 255;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}
