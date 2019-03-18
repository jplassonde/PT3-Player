// Used a bit for testing. Not currently operational.

#include "project.h"
#include "Button.h"

Button::Button(uint16_t x, uint16_t y, uint16_t xSize, uint16_t ySize, uint32_t img, uint32_t imgPressed,
			   std::function<void(uint16_t, uint16_t)> cb) : ScreenElement(x, y, xSize, ySize), img(img), pressedImg(imgPressed), callBack(cb) {
	pressed = false;
}

Button::~Button() {
}

void Button::draw() { /*
	uint32_t destination = Screen::getBackBuffer()+(800*yPos+xPos)*4;
	hdma2d.Init.Mode         = DMA2D_M2M_BLEND;
	hdma2d.Init.OutputOffset = 800 - xSize;
	hdma2d.LayerCfg[0].InputOffset = 800 - xSize;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
	hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputOffset = 0;
	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 0);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);

	HAL_DMA2D_BlendingStart_IT(&hdma2d, (pressed ? img : pressedImg), destination, destination, xSize, ySize);
	xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
	*/
}

void Button::press(TOUCH_EVENT_T touchEvent) {
	pressed = true;
}
void Button::contact(TOUCH_EVENT_T touchEvent) {

}
void Button::liftOff(TOUCH_EVENT_T touchEvent) {
	if (pressed) {
		callBack(5,5);
	}
}

bool Button::isInside(uint16_t x, uint16_t y) {

	if (x >= xPos && x <= xPos+xSize && y >= yPos && y <= yPos+ySize) {
		return true;
	} else {
		pressed = false;
		return false;
	}
}
