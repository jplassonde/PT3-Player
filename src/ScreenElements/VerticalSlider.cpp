/*
 * Slider.cpp
 *
 *  Created on: 15 Dec 2019
 *      Author: kaldg
 */

#include "VerticalSlider.h"
#include "Printer.h"
#include "Screen.h"
#include "sliderthumb.h"
#include "slider.h"

VerticalSlider::VerticalSlider(VerticalSlider_t * init)
				: ScreenElement(init->x, init->y, 80, 350) {
	slidePos = init->y + (init->ySize/2);
	pressed = false;
}

VerticalSlider::~VerticalSlider() {
	// TODO Auto-generated destructor stub
}

void VerticalSlider::draw() {

	Printer::fill(xPos, slidePos, 80, ySize - (slidePos-yPos), 0xFF990000);

	Printer::IMG_PRINT_T imgPrint = {0};
	imgPrint.imgAddress =  (uint32_t)slider80x350argb;
	imgPrint.xPosition = xPos;
	imgPrint.yPosition = yPos;
	imgPrint.imgWidth = xSize;
	imgPrint.height = ySize;
	Printer::printARGB(&imgPrint);

	imgPrint.imgAddress = (uint32_t)sliderthumb80x15argb;
	imgPrint.height = 15;
	imgPrint.yPosition = slidePos - imgPrint.height/2;
	Printer::printARGB(&imgPrint);

}

void VerticalSlider::press(TOUCH_EVENT_T touchEvent) {
	pressed = true;
}

void VerticalSlider::liftOff(TOUCH_EVENT_T touchEvent) {
	pressed = false;
}

bool VerticalSlider::isInside(uint16_t x, uint16_t y) {
    if (x >= xPos && x <= xPos + xSize && y >= yPos && y <= yPos + ySize) {
    	if(pressed) {
    		uint8_t percent = ((yPos+ySize)-y)*100/ySize;
    		update(percent);
    	}
		slidePos = y;
        return true;
    } else {
        pressed = false;
        return false;
    }
	return 0;
}

void VerticalSlider::registerFunction(void (*funct)(uint8_t)) {
	update = std::bind(funct, std::placeholders::_1);
}
