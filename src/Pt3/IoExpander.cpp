#include "Project.h"
#include "IoExpander.h"

extern I2C_HandleTypeDef ioxI2C;
extern DMA_HandleTypeDef ioxTxDMA;

constexpr uint8_t IOX_ADDR = 0x27 << 1;

IoExpander::IoExpander() {
    reset();
}

IoExpander::~IoExpander() {
    reset();
}

void IoExpander::sendData(uint8_t * data, uint16_t size) {
    SCB_CleanDCache();
    HAL_I2C_Master_Transmit_DMA(&ioxI2C, IOX_ADDR, data, size);
    xSemaphoreTake(xIOXSemaphore, portMAX_DELAY);
}

// I don't remember why this even exists
void IoExpander::sendDataPolling(uint8_t * data, uint16_t size) {
    SCB_CleanDCache();
    HAL_I2C_Master_Transmit_DMA(&ioxI2C, IOX_ADDR, data, size);
}


void IoExpander::reset() {
	// Disable the interrupt on the timers sync'd with the AY-3-8913 tone counters
	TIM3->DIER &= ~TIM_IT_UPDATE;
	TIM4->DIER &= ~TIM_IT_UPDATE;
	TIM7->DIER &= ~TIM_IT_UPDATE;

	// Reset IO expender / Soundchips
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_SET);

    // Set the period to 0x1000 (8913 counter reset value is 0, get to 0xFFF, overflow then trigger/reset to 0 again
    TIM3->ARR = 0x1000;
    TIM4->ARR = 0x1000;
    TIM7->ARR = 0x1000;
    // Reset the timers sync'd with the AY-3-8913 tone counters
    TIM3->CNT = 0;
    TIM4->CNT = 0;
    TIM7->CNT = 0;
    // 2 ms wait to make sure the IOX is operational before configuring it
    HAL_Delay(2);

    // Setup "special mode"  (Byte mode with IOCON.BANK = 0), so address pointer toggle between 2 registers of same pair
    uint8_t seqOp[] = { 0x0A, 0x20, 0x20 };
    // Setup both ports as outputs
    uint8_t data_out[] = { 0x00, 0x00, 0x00 };
    sendData(seqOp, 3);
    sendData(data_out, 3);
}

extern "C" {
void I2C1_EV_IRQHandler() {
    HAL_I2C_EV_IRQHandler(&ioxI2C);
}

void DMA1_Stream6_IRQHandler() {
    HAL_DMA_IRQHandler(&ioxTxDMA);
}

} // extern "C"
