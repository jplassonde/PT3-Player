#include "SDCard.h"
#include "IrqPriorities.h"

SD_HandleTypeDef sdHandle;
SemaphoreHandle_t xTransferCpltSema;
SDCard * sdCard;
DMA_HandleTypeDef txDMAHandle;
DMA_HandleTypeDef rxDMAHandle;

void sdInit() {
    sdCard = new SDCard();
}

SDCard::SDCard() {
    xTransferCpltSema = xSemaphoreCreateBinary();
    vQueueAddToRegistry(xTransferCpltSema, "SD DMA");

// GPIO Init
    GPIO_InitTypeDef gpioInitStruct;

    gpioInitStruct.Pin = GPIO_PIN_15; 			// SD detect line
    gpioInitStruct.Mode = GPIO_MODE_INPUT;
    gpioInitStruct.Pull = GPIO_PULLUP;
    gpioInitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOI, &gpioInitStruct);

    gpioInitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10; // DAT0 & DAT1
    gpioInitStruct.Mode = GPIO_MODE_AF_PP;
    gpioInitStruct.Speed = GPIO_SPEED_HIGH;
    gpioInitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOG, &gpioInitStruct);

    gpioInitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4; // DAT2 & DAT3
    gpioInitStruct.Alternate = GPIO_AF10_SDMMC2;
    HAL_GPIO_Init(GPIOB, &gpioInitStruct);

    gpioInitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // Clk & Cmd
    gpioInitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOD, &gpioInitStruct);

// SD & DMA Init
    __HAL_RCC_SDMMC2_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    // SD Parameters for transfer
    sdHandle.Instance = SDMMC2;
    sdHandle.Init.BusWide = SDMMC_BUS_WIDE_1B;
    sdHandle.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
    sdHandle.Init.ClockDiv = SDMMC_TRANSFER_CLK_DIV;
    sdHandle.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    sdHandle.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    sdHandle.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    sdHandle.hdmarx = &rxDMAHandle;
    sdHandle.hdmatx = &txDMAHandle;
// Configure DMA - Using DMA2 stream 0 for Rx, stream 5 for Tx

    // RX DMA config
    rxDMAHandle.Instance = DMA2_Stream0;
    rxDMAHandle.Init.Channel = DMA_CHANNEL_11;
    rxDMAHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    rxDMAHandle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    rxDMAHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    rxDMAHandle.Init.MemBurst = DMA_MBURST_INC4;
    rxDMAHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    rxDMAHandle.Init.MemInc = DMA_MINC_ENABLE;
    rxDMAHandle.Init.Mode = DMA_PFCTRL;
    rxDMAHandle.Init.PeriphBurst = DMA_PBURST_INC4;
    rxDMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    rxDMAHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    rxDMAHandle.Init.Priority = DMA_PRIORITY_HIGH;
    rxDMAHandle.Parent = &sdHandle;

    // Tx DMA config
    txDMAHandle.Instance = DMA2_Stream5;
    txDMAHandle.Init.Channel = DMA_CHANNEL_11;
    txDMAHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    txDMAHandle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    txDMAHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    txDMAHandle.Init.MemBurst = DMA_MBURST_INC4;
    txDMAHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    txDMAHandle.Init.MemInc = DMA_MINC_ENABLE;
    txDMAHandle.Init.Mode = DMA_PFCTRL;
    txDMAHandle.Init.PeriphBurst = DMA_PBURST_INC4;
    txDMAHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    txDMAHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    txDMAHandle.Init.Priority = DMA_PRIORITY_HIGH;
    txDMAHandle.Parent = &sdHandle;

    // Set Interrupts priority and enable them
    NVIC_SetPriority(DMA2_Stream0_IRQn, SDMARX_PRIO);
    NVIC_SetPriority(DMA2_Stream5_IRQn, SDMATX_PRIO);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    NVIC_SetPriority(SDMMC2_IRQn, SD_IRQ_PRIO);
    NVIC_EnableIRQ(SDMMC2_IRQn);

    // Init DMA streams
    HAL_DMA_Init(&txDMAHandle);
    HAL_DMA_Init(&rxDMAHandle);

    // Init SD card using HAL
    HAL_SD_Init(&sdHandle);

    if (HAL_SD_ConfigWideBusOperation(&sdHandle, SDMMC_BUS_WIDE_4B) != HAL_OK) {
        while (1); // Hang-debug
    }
}

SDCard::~SDCard() {
}

// function called in fatfs diskio.cpp ------------------------------------------------

DSTATUS SDCard::getStatus() {
    if (sdHandle.State == HAL_SD_STATE_RESET) {
        return STA_NOINIT;
    } else {
        return 0;
    }
}

DRESULT SDCard::sdRead(uint8_t * buff, uint32_t firstSector, uint32_t sectorCount) {
    HAL_StatusTypeDef result;
    SCB_CleanInvalidateDCache();

    result = HAL_SD_ReadBlocks_DMA(&sdHandle, buff, firstSector, sectorCount);
    xSemaphoreTake(xTransferCpltSema, portMAX_DELAY);

    // Tell the CPSM to go back in Idle state. ST lib forgets to do it,
    // and would cause a timeout if 3 mins elapse between transfers.
    sdHandle.Instance->DCTRL |= SDMMC_DCTRL_RWSTOP;

    if (result == HAL_OK) {
        return RES_OK;
    } else {
        return RES_ERROR;
    }
}

// IRQ Handlers ------------------------------------------------------------------------

extern "C" {

void HAL_SD_RxCpltCallback(SD_HandleTypeDef * sdh) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xTransferCpltSema, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

void SDMMC2_IRQHandler() {
    HAL_SD_IRQHandler(&sdHandle);
}

void DMA2_Stream0_IRQHandler() {
    HAL_DMA_IRQHandler(sdHandle.hdmarx);

}

void DMA2_Stream5_IRQHandler() {
    HAL_DMA_IRQHandler(sdHandle.hdmatx);
}
}
