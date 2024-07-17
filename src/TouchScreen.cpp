#include "project.h"
#include "TouchScreen.h"
#include "stdlib.h"

extern QueueHandle_t xTSEventQueue;

SemaphoreHandle_t xTsIntSemaphore;
SemaphoreHandle_t xTsI2CSemaphore;
SemaphoreHandle_t xI2C4Mutex; // Will be useful later, same line as the audio codec

constexpr uint8_t TS_ADDR = 0x54;
constexpr uint8_t RXBUFFERSIZE = 5;
constexpr uint8_t GESTURE_THRESHOLD = 15;
constexpr uint8_t DRIFT_LIM = 10;

TouchScreen::TouchScreen() {}

TouchScreen::~TouchScreen() {}

void TouchScreen::monitorTs() {
    uint8_t rxBuffer[RXBUFFERSIZE] = { 0 };
    TOUCH_EVENT_T touchData = { 0 };

    while (1) {
        xSemaphoreTake(xTsIntSemaphore, portMAX_DELAY);
        setAddress();
        getTsData(rxBuffer);

        touchData.touchEvent = rxBuffer[0] >> 6;
        /* Todo: Filtering for Y axis. Badly calibrated. TS controller autocalibration is
         apparently not working & undocumented. */
        touchData.yPosition = 479 - (((rxBuffer[0] & 0x0F) << 8) | rxBuffer[1]);
        touchData.xPosition = (((rxBuffer[2] & 0x0F) << 8) | rxBuffer[3]);
        findGesture(&touchData);
        xQueueSend(xTSEventQueue, (void * )&touchData, portMAX_DELAY);
    }
}

void TouchScreen::setAddress() {
    uint8_t setDataAddr[] = { 0x03 };

    xSemaphoreTake(xI2C4Mutex, portMAX_DELAY);

    HAL_I2C_Master_Transmit_IT(&i2c4, TS_ADDR, setDataAddr, 1);
    xSemaphoreTake(xTsI2CSemaphore, portMAX_DELAY);

    xSemaphoreGive(xI2C4Mutex);
}

void TouchScreen::getTsData(uint8_t * rxBuffer) {
    xSemaphoreTake(xI2C4Mutex, portMAX_DELAY);

    HAL_I2C_Master_Receive_IT(&i2c4, TS_ADDR, rxBuffer, RXBUFFERSIZE);
    xSemaphoreTake(xTsI2CSemaphore, portMAX_DELAY);

    xSemaphoreGive(xI2C4Mutex);
}

// The version of the TS controller used on the ST board apparently does not support gestures.
// Lets add it with software...
void TouchScreen::findGesture(TOUCH_EVENT_T * touchData) {
    static uint16_t lastX = touchData->xPosition;
    static uint16_t lastY = touchData->yPosition;

    int16_t diffX = 0;
    int16_t diffY = 0;

    touchData->gesture = NO_GESTURE;

    if (touchData->touchEvent == EVENT_CONTACT) {
        diffX = touchData->xPosition - lastX;
        diffY = touchData->yPosition - lastY;
        if (abs(diffX) >= GESTURE_THRESHOLD && abs(diffY) <= DRIFT_LIM) {
            (diffX > 0) ?
                    touchData->gesture = SWIPE_RIGHT : touchData->gesture = SWIPE_LEFT;
            touchData->gestureMagnitude = (uint16_t)abs(diffX);
            lastX = touchData->xPosition;
            lastY = touchData->yPosition;
        } else if (abs(diffY) >= GESTURE_THRESHOLD && abs(diffX) <= DRIFT_LIM) {
            (diffY > 0) ?
                    touchData->gesture = SWIPE_DOWN : touchData->gesture = SWIPE_UP;
            touchData->gestureMagnitude = (uint16_t)abs(diffY);
            lastX = touchData->xPosition;
            lastY = touchData->yPosition;
        } else {
            touchData->gesture = NO_GESTURE;
        }
    } else if (touchData->touchEvent == EVENT_DOWN) {
        lastX = touchData->xPosition;
        lastY = touchData->yPosition;
    }
}

//------------------ Task Entry ------------------//

void TouchEventTask(__attribute__((unused)) void *pvParameters) {
    xTsIntSemaphore = xSemaphoreCreateBinary();

    TouchScreen ts;
    ts.monitorTs();
}

//------- Interrupts & Callbacks Handlers -------//

extern "C" {
void I2C4_EV_IRQHandler() {
    HAL_I2C_EV_IRQHandler(&i2c4);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef * hi2c) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }

    if (hi2c->Instance == I2C4) {
        xSemaphoreGiveFromISR(xTsI2CSemaphore, &xHigherPriorityTaskWoken);
    }

    if (hi2c->Instance == I2C1) {
        xSemaphoreGiveFromISR(xIOXSemaphore, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * hi2c) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }

    if (hi2c->Instance == I2C4) {
        xSemaphoreGiveFromISR(xTsI2CSemaphore, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken != pdFALSE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void EXTI15_10_IRQHandler() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);

        xSemaphoreGiveFromISR(xTsIntSemaphore, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken != pdFALSE) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }


}
}
