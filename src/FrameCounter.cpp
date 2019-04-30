#include <FrameCounter.h>
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

FrameCounter::FrameCounter() {
    static bool isTimerInit = 0;
    count = 0;
    counter = 0;
    if (!isTimerInit) {

        __HAL_RCC_TIM11_CLK_ENABLE();

        TIM_HandleTypeDef timer;
        timer.Instance = TIM11;
        timer.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
        timer.Init.CounterMode = TIM_COUNTERMODE_UP;
        timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        timer.Init.Prescaler = 43199;
        timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        timer.Init.Period = 4999;
        HAL_TIM_Base_Init(&timer);

        TIM_ClockConfigTypeDef timerClock;
        timerClock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        HAL_TIM_ConfigClockSource(&timer, &timerClock);

        NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
        HAL_TIM_Base_Start_IT(&timer);
    }
    isTimerInit = true;
}

FrameCounter::~FrameCounter() {
    // TODO Auto-generated destructor stub
}

void FrameCounter::latchCounter() {
    count = counter + 0;
    counter = 0;
}

void FrameCounter::incCounter() {
    counter++;
}

uint8_t FrameCounter::getCount() {
    return count;
}

