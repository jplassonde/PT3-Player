#include "project.h"
#include "FrameCounter.h"

extern FrameCounter * fc;
extern FrameCounter * vbc;

#ifdef __cplusplus
extern "C" {
#endif

volatile uint8_t msFlag = 0;

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
    while (1);
}

void HardFault_Handler() {
    while (1);
}
void MemManage_Handler() {
    while (1);
}
void BusFault_Handler() {
    while (1);
}
void UsageFault_Handler() {
    while (1);
}

void OTM8009A_IO_Delay(uint32_t Delay) {
    HAL_Delay(Delay);
}

void SysTick_Handler() {
    HAL_IncTick();

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}

void DMA2D_IRQHandler() {
    HAL_DMA2D_IRQHandler(&hdma2d);
}

void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef * hdsi) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vbc->incCounter();
        if (xSemaphoreTake(xVblankSema, 0) == pdTRUE) {
            HAL_DSI_Refresh(hdsi);
        }
    }
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED
            && xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef * hdsi) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xSemaphoreGiveFromISR(xDsiSemaphore, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void DSI_IRQHandler() {
    HAL_DSI_IRQHandler(&hdsi);
}

// 1 Hz timer, for FPS
void TIM1_TRG_COM_TIM11_IRQHandler() {
    TIM11->SR = ~TIM_FLAG_UPDATE;
    fc->latchCounter();
    vbc->latchCounter();
}

// 50 Hz timer, for soundchips update
void TIM2_IRQHandler() {
    CLEAR_BIT(TIM2->SR, TIM_FLAG_UPDATE);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xSemaphoreGiveFromISR(xPlayTickSema, &xHigherPriorityTaskWoken);

        if (xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}


void DSI_IO_WriteCmd(uint32_t NbrParams, uint8_t *pParams) {
    if (NbrParams <= 1) {
        HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, pParams[0],
                pParams[1]);
    } else {
        HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, NbrParams,
                pParams[NbrParams], pParams);
    }
}

#ifdef __cplusplus
}
#endif

void XferCpltCallback(struct __DMA2D_HandleTypeDef * hdma2d) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xDma2dSemaphore, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
