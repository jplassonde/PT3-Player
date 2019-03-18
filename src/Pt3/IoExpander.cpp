#include "Project.h"
#include "IoExpander.h"


extern I2C_HandleTypeDef ioxI2C;

constexpr uint8_t IOX_ADDR = 0x27 << 1;
constexpr uint8_t TIMEOUT = 20;

IoExpander::IoExpander() {
	reset();
}

IoExpander::~IoExpander() {
	reset();
}

void IoExpander::sendData(uint8_t * data, uint8_t size) {
	HAL_I2C_Master_Transmit_IT(&ioxI2C, IOX_ADDR, data, size); // TODO: DMA. Proper DMA. Not HAL DMA.
	xSemaphoreTake(xIOXSemaphore, portMAX_DELAY);
}

void IoExpander::reset() {
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_Delay(2);
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(2);

	// Setup "special mode"  (Byte mode with IOCON.BANK = 0), so address pointer toggle between 2 registers of same pair
	uint8_t seqOp[] = {0x0A, 0x20, 0x20};
	// Setup both ports as outputs
	uint8_t data_out[] = {0x00, 0x00, 0x00};
	sendData(seqOp, 3);
	sendData(data_out, 3);
}

extern "C" {
void I2C1_EV_IRQHandler() {
	HAL_I2C_EV_IRQHandler(&ioxI2C);
}
}
