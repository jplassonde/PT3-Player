#pragma once

// LCD Related
#define LTDC_PRIO		12
#define DMA2D_PRIO		12
#define DSI_PRIO		12
#define TS_INT_PRIO		12
#define TS_I2C_PRIO		12

// SD Card related
#define SD_IRQ_PRIO		14 // SDMMC2 Interrupt
#define SDMARX_PRIO		15 // RX DMA Stream IRQ (DMA2 Stream 0)
#define SDMATX_PRIO		15 // TX DMA Stream IRQ (DMA2 Stream 5)

// AY Related
#define AY_VBLANK_PRIO	10
#define IOX_I2C_PRIO	7
#define IOX_DMA_PRIO 	10

// AY-3-8913 Sync timers
#define CHAN_A_TIM_PRIO 8
#define CHAN_B_TIM_PRIO 8
#define CHAN_C_TIM_PRIO 8

// SAI Related
#define SAI_DMA_PRIO 7

// USB
#define OTG_HS_IRQ_PRIO 6

