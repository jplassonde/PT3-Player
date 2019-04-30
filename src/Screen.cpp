#include "Screen.h"
#include "otm8009a.h"
#include "project.h"
#include "fbState.h"

extern SemaphoreHandle_t xVblankSema;
extern SemaphoreHandle_t xDsiSemaphore;

// Commands to set active columns to draw on the DSI
const uint8_t leftHalf[] = { 0x00, 0x00, 0x01, 0x8F, OTM8009A_CMD_CASET };
const uint8_t rightHalf[] = { 0x01, 0x90, 0x03, 0x1F, OTM8009A_CMD_CASET };

// Initilization of the "private" static variables
uint32_t Screen::backBuffer = BUFFER1;
uint32_t Screen::frontBuffer = BUFFER2;

/*********************************************************************************
 Method: Screen::refresh
 Description:
 Handle screen refresh. The left half of the screen is sent to the display
 display GRAM as soon as the next VSYNC signal is received
 (handled in HAL_DSI_TearingEffectCallback, when xVblankSema is given),
 then the right half is updated.
 The display is configured to send the tearing effect when it reaches the 400th line.
 This is necessary since the display refresh from "left to right" and the GRAM updates
 from "top to bottom" in landscape mode.

 ***********************************************************************************/
void Screen::refresh() {
    __HAL_DSI_WRAPPER_DISABLE(&hdsi);
    LTDC_LAYER(&hltdc, 0)->CFBAR = frontBuffer;
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    __HAL_DSI_WRAPPER_ENABLE(&hdsi);
    DSI_IO_WriteCmd(4, (uint8_t *)leftHalf);

    xSemaphoreGive(xVblankSema);
    xSemaphoreTake(xDsiSemaphore, portMAX_DELAY);

    __HAL_DSI_WRAPPER_DISABLE(&hdsi);
    LTDC_LAYER(&hltdc, 0)->CFBAR = frontBuffer + 400 * 4;
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    __HAL_DSI_WRAPPER_ENABLE(&hdsi);
    DSI_IO_WriteCmd(4, (uint8_t *)rightHalf);
    HAL_DSI_Refresh(&hdsi);

    xSemaphoreTake(xDsiSemaphore, portMAX_DELAY);
}

void Screen::swapBuffers() {
    uint32_t temp = backBuffer;
    backBuffer = frontBuffer;
    frontBuffer = temp;
}

uint32_t Screen::getBackBufferAddr() {
    return backBuffer;
}

/************************************************************************************
 Method: Screen::DisplayTask
 Description:
 Main loop for the task.
 Wait for the backbuffer to be drawn,
 swap the front and back buffers,
 send signal that the backbuffer is available for writing,
 then update the display.

 *************************************************************************************/
void DisplayTask(__attribute__((unused)) void *pvParameters) {
    while (1) {
        xEventGroupWaitBits(xFramebuffersState, BB_DRAWN, pdTRUE, pdTRUE, portMAX_DELAY);
        Screen::swapBuffers();
        xEventGroupSetBits(xFramebuffersState, BB_AVAILABLE);
        Screen::refresh();
    }
}
