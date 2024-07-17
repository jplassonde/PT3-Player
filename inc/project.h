#pragma once

// ST Headers
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

// FreeRTOS headers
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "IrqPriorities.h"

// Global Handles
extern I2C_HandleTypeDef ioxI2C;
extern LTDC_HandleTypeDef  hltdc;
extern I2C_HandleTypeDef i2c4;
extern DMA2D_HandleTypeDef hdma2d;
extern DSI_HandleTypeDef hdsi;

// Global FreeRTOS sync'ers
extern SemaphoreHandle_t xDma2dSemaphore;
extern SemaphoreHandle_t xDsiSemaphore;
extern SemaphoreHandle_t xVblankSema;
extern SemaphoreHandle_t xPlayTickSema;
extern SemaphoreHandle_t xIOXSemaphore;
extern SemaphoreHandle_t xI2C4Mutex;
extern SemaphoreHandle_t xTsI2CSemaphore;
extern EventGroupHandle_t xFramebuffersState;
extern QueueHandle_t xTSEventQueue;
