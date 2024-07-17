#include "project.h"
#include "Printer.h"
#include "Screen.h"
#include <cmath>
#include "colors.h"

extern const uint8_t sinetable[];
extern uint32_t globalTick;

// Byte per pixel
constexpr uint8_t bppIn = 3; // Bytes per pixel of source images (all but fonts).
constexpr uint8_t bppOut = 4; // Bytes per pixel of screenbuffer.

namespace Printer {

namespace {
void prvPrintCroppedChar(uint16_t offset, char c, uint8_t leftCrop, uint8_t rightCrop, STR_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr()
            + (ps->yPosition * 800 + ps->xPosition + offset) * bppOut;
    uint8_t width = ps->font->getWidth();

    hdma2d.Init.Mode = DMA2D_M2M_BLEND;
    hdma2d.Init.OutputOffset = 800 - (width - leftCrop - rightCrop);
    hdma2d.LayerCfg[0].InputOffset = 800 - (width - leftCrop - rightCrop);
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A8;
    hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = leftCrop + rightCrop;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 0);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    HAL_DMA2D_BlendingStart_IT(&hdma2d,
            (uint32_t)ps->font->getChar(c) + leftCrop + ps->topCrop * width,
            destination, destination, width - leftCrop - rightCrop, ps->height);
    xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

void prvPrintColChar(uint16_t offset, char c, uint8_t leftCrop, uint8_t rightCrop, STR_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr()
            + (ps->yPosition * 800 + ps->xPosition + offset) * bppOut;
    const uint8_t * source = ps->font->getChar(c);

    for (int h = 0; h < ps->font->getHeight(); h++) {
        for (int w = 0; w < (ps->font->getWidth() - leftCrop - rightCrop);
                w++) {
            if (source[h * ps->font->getWidth() + leftCrop + w] == 0xFF) {
                *(uint32_t *)(destination + bppOut * (h * 800 + w)) = getColor(
                        ps->xPosition + w + offset, ps->yPosition + h);
            }
        }
    }
}
} // End Anon Namespacewq

void printString(STR_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr() + (ps->yPosition * 800 + ps->xPosition) * 4;
    int i = 0;

    hdma2d.Init.Mode = DMA2D_M2M_BLEND;
    hdma2d.Init.OutputOffset = 800 - ps->font->getWidth();
    hdma2d.LayerCfg[0].InputOffset = 800 - ps->font->getWidth();
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A8;
    hdma2d.LayerCfg[1].InputAlpha = 0xFFCFCFCF;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = 0;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 0);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    while (ps->str[i]) {
        HAL_DMA2D_BlendingStart_IT(&hdma2d,
                (uint32_t)(ps->font->getChar(ps->str[i])), destination,
                destination, ps->font->getWidth(), ps->font->getHeight());

        destination += ps->font->getWidth() * bppOut;
        ++i;
        xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
    }
}

void printColString(STR_PRINT_T * ps) {
    uint8_t width = ps->font->getWidth();
    uint32_t destination = Screen::getBackBufferAddr() + (ps->yPosition * 800 + ps->xPosition) * 4;
    uint32_t workDest;
    uint32_t offset = 0;
    uint8_t i = 0;
    const uint8_t * source;

    while (ps->str[i]) {
        if (ps->str[i] != '\x20') { // Skip spaces
            workDest = destination + 4 * offset;
            source = ps->font->getChar(ps->str[i]);
            for (int h = 0; h < width; h++) {
                for (int w = 0; w < width; w++) {
                    if (source[width * h + w] == 0xFF) {
                        *(uint32_t *)(workDest + 4 * w) = getColor(ps->xPosition + w + offset, ps->yPosition + h);
                    }
                }
                workDest += 4 * 800;
            }
        }
        offset += width;
        ++i;
    }
}

void printCroppedString(STR_PRINT_T * ps) {
    uint8_t leftCrop = ps->leftCrop % ps->font->getWidth(); // How much of the first character printed need to be left cropped
    uint16_t charIndex = (uint8_t)floor(ps->leftCrop / ps->font->getWidth()); // First character to print
    uint32_t widthRemaining = ps->length;		// Max amount of width to print
    uint8_t rightCrop = 0; 	// Right crop
    uint16_t offset = 0;

    // Print characters until it needs right-cropping
    while (ps->str[charIndex] && widthRemaining) {
        rightCrop =
                (widthRemaining < ps->font->getWidth() ? ps->font->getWidth() - widthRemaining : 0);

        if (ps->color) {
            prvPrintColChar(offset, ps->str[charIndex], leftCrop, rightCrop, ps);
        } else {
            prvPrintCroppedChar(offset, ps->str[charIndex], leftCrop, rightCrop, ps);
        }
        offset += (ps->font->getWidth() - leftCrop);
        ++charIndex;
        widthRemaining -= ps->font->getWidth() - leftCrop - rightCrop;
        leftCrop = 0;
    }
}

void printImg(IMG_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr() + (ps->yPosition * 800 + ps->xPosition);

    hdma2d.Init.Mode = DMA2D_M2M_PFC;
    hdma2d.Init.OutputOffset = 800 - ps->imgWidth;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
    hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    HAL_DMA2D_Start_IT(&hdma2d, ps->imgAddress, destination, ps->imgWidth, ps->height);

    xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

void printARGB(IMG_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr() + (ps->yPosition * 800 + ps->xPosition) * bppOut;

    hdma2d.Init.Mode = DMA2D_M2M_BLEND;
    hdma2d.Init.OutputOffset = 800 - ps->imgWidth;
    hdma2d.LayerCfg[0].InputOffset = 800 - ps->imgWidth;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 0);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    HAL_DMA2D_BlendingStart_IT(&hdma2d, ps->imgAddress, destination, destination, ps->imgWidth, ps->height);
    xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

void printCroppedImg(IMG_PRINT_T * ps) {
    uint32_t destination = Screen::getBackBufferAddr()
            + (ps->yPosition * 800 + ps->xPosition) * bppOut;
    uint32_t source = ps->imgAddress
            + (ps->topCrop * ps->imgWidth + ps->leftCrop) * bppIn;

    hdma2d.Init.Mode = DMA2D_M2M_PFC;
    hdma2d.Init.OutputOffset = 800 - ps->imgWidth;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
    hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = ps->leftCrop + ps->rightCrop;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    HAL_DMA2D_Start_IT(&hdma2d, source, destination, ps->imgWidth - ps->leftCrop - ps->rightCrop, ps->height);

    xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

void fill(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t color) {
    uint32_t destination = Screen::getBackBufferAddr()
            + (y * 800 + x) * bppOut;
	hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.OutputOffset = 800 - xSize;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputOffset = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
    HAL_DMA2D_Start_IT(&hdma2d, color, destination, xSize, ySize);
    xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}

uint32_t getColor(uint16_t x, uint16_t y) {
    return color2[(((sinetable[((x >> 2) + (globalTick << 3)) & 0xFF] << 1)
            + (y << 1) + (globalTick << 3)) >> 2) & 0xFF];
}
}
