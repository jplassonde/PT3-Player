#include "MainEngine.h"
#include "States.h"
#include "fbState.h"
#include "SDCard.h"

// Gfx
#include "scanline.h"
#include "speccy800x180.h"

// Global variables
uint32_t globalTick;
Font * rezFont18px;
Font * rezFont27px;
SDCard * sdCard; // Doesnt go here, but have to be created once the scheduler is started
				 // and kind of need to be global since fatFS is using it.

// FreeRTOS
extern QueueHandle_t xTSEventQueue;

MainEngine::MainEngine()  {
	rezFont18px = new Font(Font::rez18px); // Out of place...
	rezFont27px = new Font(Font::rez27px);
	currentState = new Idle(this);
	globalTick = 0;
}

MainEngine::~MainEngine() {
	delete rezFont18px;
	delete rezFont27px;
	delete currentState;
}
/*********************************************************************************
Method: MainEngine::run
Description:
Main program loop. Get touch data from the queue, sent from the touchScreen object
and pass it to the active "window" for processing

Sync through an event group with the task responsible for refreshing the display
to signal the backbuffer is ready and to wait for the backbuffer to be available
**********************************************************************************/
void MainEngine::run() {
	TOUCH_EVENT_T touchEvent;

	while(1) {
		if (xQueueReceive(xTSEventQueue, (void*)&touchEvent, 0) == pdTRUE) {
			currentState->processTouch(touchEvent);
		}
		xEventGroupWaitBits(xFramebuffersState, BB_AVAILABLE, pdTRUE, pdTRUE, portMAX_DELAY);
		currentState->drawScreen();
		xEventGroupSetBits(xFramebuffersState, BB_DRAWN);
		++globalTick;
	}
}

void MainEngine::switchState(AbstractState * state) {
	currentState = state;
}

/**********************************************************************************
Method: MainEngine::addScanlines
Description:
Add horizontal scanlines to the display.

Whoever designed the WVGA display forgot to put scanlines in it, so I'm adding them
here.
***********************************************************************************/
void MainEngine::addScanlines() {
	hdma2d.Init.Mode         = DMA2D_M2M_BLEND;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[0].InputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_A8;
	hdma2d.LayerCfg[1].InputAlpha = 0x8F000000;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
	hdma2d.LayerCfg[1].InputOffset = 0;

	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 0);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);

	for (int i = 0; i<480; i++) {
		if ((i % 2) == 0) {
			HAL_DMA2D_BlendingStart_IT(&hdma2d, (uint32_t)scanline,
					Screen::getBackBufferAddr()+800*i*4,
					Screen::getBackBufferAddr()+800*i*4, 800, 1);
			xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);

		}
	}
}

/**********************************************************************************
Method: MainEngine::addScanlines
Description:
Add horizontal scanlines to the display.

Whoever designed the WVGA display forgot to put scanlines in it, so I'm adding them
here.
***********************************************************************************/
void MainEngine::drawBackground() {
	hdma2d.Init.Mode = DMA2D_R2M;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputOffset = 0;
	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	HAL_DMA2D_Start_IT(&hdma2d, 0xFF313131, Screen::getBackBufferAddr(), 800, 300);
	xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);

	hdma2d.Init.Mode         = DMA2D_M2M_PFC;
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
	HAL_DMA2D_Init(&hdma2d);
	HAL_DMA2D_ConfigLayer(&hdma2d, 1);
	HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)speccy800x180, Screen::getBackBufferAddr()+800*300*4, 800, 180);
	xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
}



// Task Entry
void MainTask(__attribute__((unused)) void *pvParameters) {
	sdCard = new SDCard();
	MainEngine * mainEngine = new MainEngine();
	mainEngine->run();
}
