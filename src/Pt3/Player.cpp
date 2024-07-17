#include "Player.h"
#include <cstring>
#include "Project.h"
#include "TrackInfos.h"
#include "IEvent.h"
#include "TrackTime.h"
#include "ControlReq.h"

extern QueueHandle_t xPlayerCmdQueue;
extern QueueHandle_t xIeQueue;
extern QueueHandle_t xUsbhCtrlQueue;
extern SemaphoreHandle_t xPlayTickSema;

extern TrackTime * trackTime;

Player::Player() {
    iox = new IoExpander();
    txArr[0] = IOA;
    playing = false;
    module = nullptr;
    folder = nullptr;
    loopTime = 0;
}

Player::~Player() {
    delete iox;
    delete module;
}

void Player::run() {

    PLAYER_QUEUE_T pq;

    while (1) {
        if (xQueueReceive(xPlayerCmdQueue, (void*)&pq, 0) == pdTRUE) { // check queue between each tick. Will be useful
            processQueue(&pq);	// when other commands (pause, etc...) are added
        }

        if (playing) {
            if (play() == TRACK_FINISHED) {
                //loadNext();
            	trackTime->setCurrent(loopTime);
            }
        } else {
        	xQueuePeek(xPlayerCmdQueue, (void*)&pq, portMAX_DELAY);  // Hang until queue receive if not playing
        }
    }
}

void Player::processQueue(PLAYER_QUEUE_T * pq) {

    switch (pq->cmd) {

        case PLAY:
            delete folder;
            folder = pq->folder;
            loadModule();
            break;

        case PAUSE:
            playing = !playing;
            break;

        case FF: {
            for (auto &i : parsers) {
                i->reset();
            }
            uint32_t currentTime = 0;
            while (currentTime < pq->pos) {
                for (auto &i : parsers) {
                    i->processTick();
                }
                currentTime += 20;
            }
            trackTime->setCurrent(currentTime);
        }
            break;

        case STOP:
        	resetParsers();
        	playing = !playing;
        	// Send signal to UI
        	break;

        default:
            while(1); //something is broken, debug.
    }
}

void Player::initParser(uint8_t chipMask, uint8_t * modAddress) {
    const uint8_t lastStateInit[13] = { 0 };
    std::unique_ptr<CHIPSTATE_T> chip((CHIPSTATE_T*)pvPortMalloc(sizeof(CHIPSTATE_T)));
    memcpy(&chip->last, &lastStateInit, 13);
    chip->current.mask = chipMask;

    std::unique_ptr<Pt3Parser> parser(new Pt3Parser(&(chip->current), modAddress));

    chipV.push_back(std::move(chip));
    parsers.push_back(std::move(parser));
}

uint8_t Player::play() {
    uint16_t length = 1; // Total message length, first byte is IoX address.
    bool trackStatus = !TRACK_FINISHED;

    for (auto &i : parsers) {
        trackStatus = i->processTick();
    }

    if (trackStatus == TRACK_FINISHED) {
    	return trackStatus;
    }

    for (auto &i : chipV) {
        length = buildArray(length, (uint8_t *)&i->current, (uint8_t *)&i->last,
                i->current.mask);
    }

    xSemaphoreTake(xPlayTickSema, portMAX_DELAY);
    trackTime->incCurrent();

    if (length > 5) { // Don't bother sending if there is no register update
        iox->sendData(txArr, length);
    }

    updateLeds();

    for (auto &i : chipV) {
    	if (i->current.mask == AY13) {
    	    updateAY13Tone((uint8_t *)&i->current, (uint8_t *)&i->last);
    	}
        memcpy(&i->last, &i->current, sizeof(SOUNDCHIP_T));
    }

    return trackStatus;
}

void Player::updateLeds() {
	ControlReq * ctrlReq;
	ControlReq::Init_t CtrlInit;
	uint8_t buff[20] = {0};

	CtrlInit.bmRT = 0x21; // Type class - recipient interface
	CtrlInit.bReq = 0x09; // Set report
	CtrlInit.wVal = 0x211; // Output - Report id 0x11
	CtrlInit.wIdx = 3;
	CtrlInit.wLen = 20;
	CtrlInit.dat = buff;

	buff[0] = 0x11;
	buff[1] = 0xFF;
	buff[2] = 0x04;
	buff[3] = 0x3C;
	buff[4] = 0x01;
	buff[5] = 0x01;
	buff[9] = 0x02;

	auto i = chipV[0].get();
	i->current.bVal = 0;
	i->current.rVal = 0;
	uint8_t mask = i->current.mixer;

	if ((mask & 0x08) == 0) {
		i->current.rVal += i->current.volA;
	} else {
		i->current.bVal += (mask & 0x01) ? 0 : i->current.volA;
	}

	if ((mask & 0x10) == 0) {
		i->current.rVal += i->current.volB;
	} else {
	//	i->current.bVal += (mask & 0x02) ? 0 : i->current.volB & 0x0F;
	}

	if ((mask & 0x20) == 0) {
		i->current.rVal += i->current.volC;
	} else {
		//i->current.bVal += (mask & 0x03) ? 0 : i->current.volC & 0x0F;
	}

	if (i->current.bVal == i->last.bVal && i->current.rVal == i->last.rVal) {
		return; // dont update if there is no update to do...
	}
	buff[6] = i->current.rVal * 2;
	buff[8] = i->current.bVal * 3;

	ctrlReq = new ControlReq(&CtrlInit);
	if (xQueueSend(xUsbhCtrlQueue, &ctrlReq, 0) != pdTRUE) {
		delete ctrlReq;
	}
}

void Player::loadNext() {
    folder->advanceFile();
    loadModule();
}

void Player::loadModule() {
	resetParsers();
    FILE_DATA_t fileData;
    fileData = folder->getFileData();
    module = fileData.data;

    TrackInfos * infos = new TrackInfos(folder->getFilename(),
            (const char *)&module[30], (const char *)&module[66]);
    IEVENT_t iEvent = { PLAYING, infos };
    xQueueSend(xIeQueue, (void * )&iEvent, portMAX_DELAY);

    initParser(AY30, module);

    if (strncmp((const char *)&module[fileData.size - 4], "02TS", 4) == 0) { // Turbosound signature
        uint16_t secondModIndex = *(uint16_t *)&module[fileData.size - 12]; // Get starting index for 2nd module
        initParser(AY13, &module[secondModIndex]);
    }

    trackTime->setTotal(parsers[0]->getTotalTime());
    trackTime->setCurrent(0);
    loopTime = parsers[0]->getLoopTime();
    playing = true;
}

uint8_t Player::buildArray(uint16_t index, uint8_t * current, uint8_t * previous, uint8_t mask) {
    uint8_t latchAddr = mask | BDIR | BC1;
    uint8_t writeData = mask | BDIR;
    uint8_t busInactive = mask;

    // Send updated data using one long transfer with bus transactions, IO expander will alternate between ports
    // Value at index 15 indicate a reset of the envelope period -> needs to be updated even if it has the same value

    for (int i = 0; i <= 13; i++) {
       if (current[i] != previous[i] || (i == 13 && current[15] == 1)) {

    	   if (mask != AY13 || i > 5) { // Do not update the tone channels of the AY-3-8913 here, because defect
				txArr[index++] = busInactive;
				txArr[index++] = i;
				txArr[index++] = latchAddr;
				txArr[index++] = i;
				txArr[index++] = busInactive;
				txArr[index++] = current[i];
				txArr[index++] = writeData;
				txArr[index++] = current[i];
    	   }
       }
    }

    txArr[index++] = busInactive;
    txArr[index++] = 0;
    current[15] = 0;
    return index;
}

/*************************************************************************
 * Special methods to update the AY-3-8913 tone register.
 * -----------------------------------------------------------------------
 * The chip is a factory reject with a major defect on the tone counter.
 * It only reset the period on COUNT == PERIOD and not COUNT >= PERIOD
 * so if the period is changed for a smaller value while the count is already
 * above it, it wont change the period until it overflow and go back to the
 * period value.
 *
 * 3 timers in the MCU are set to be sync'd with the tone counter, so we can
 * time the tone period update right after the count reset to 0, which should
 * work around the problem (unless the tone is set to VERY small values)
 */

void Player::updateAY13Tone(uint8_t * current, uint8_t * previous) {
    constexpr uint8_t smallerAllowedVal = 5;
    for (int i = 0; i <= 4; i+=2) { // 0-1, 2-3, 4-5 are the tone registers pairs

        // Make sure the period are never too small to allow the whole write cycle/sync
    	if (*(uint16_t*)&current[i] < smallerAllowedVal) { //
    		*(uint16_t*)&current[i] = smallerAllowedVal;
    	}

    	// Update the tone register if needed
        if (*(uint16_t*)&previous[i] != *(uint16_t*)&current[i]) {
        	updateAY13Chan(&current[i], &previous[i], i);
        }
    }
}

constexpr uint8_t LA = AY13 | BDIR | BC1;
constexpr uint8_t WD = AY13 | BDIR;
constexpr uint8_t BI = AY13;

typedef struct Ay13Data_t {
		uint8_t txBuffer[12];
		uint8_t txCount;
		uint16_t timerVal;
} Ay13Data_t;

Ay13Data_t ayData = {IOA, WD, 0, BI, 0, LA, 0, BI, 0, WD, 0, BI, 0};


void Player::updateAY13Chan(uint8_t * current, uint8_t * previous, uint8_t baseReg) {

    // To preload address on the chip + preload data on the IoX.
    uint8_t prepBuffer[] = {IOA, BI, baseReg, LA, baseReg, BI, current[0]};
    if (current[0] != previous[0]) {
		if (current[1] != previous[1]) { 	// Fine & coarse tone reg need update
			ayData.txBuffer[2] = current[0];
			ayData.txBuffer[4] = baseReg+1;
			ayData.txBuffer[6] = baseReg+1;
			ayData.txBuffer[8] = current[1];
			ayData.txBuffer[10] = current[1];
			ayData.txCount = 13;
		} else {							// Only fine tone need update
			ayData.txBuffer[2] = current[0];
			ayData.txCount = 5;
		}
	} else {								// Only coarse tone need update
		prepBuffer[2] += 1;
		prepBuffer[4] += 1;
		prepBuffer[6] = current[1];

		ayData.txBuffer[2] = current[1];
		ayData.txBuffer[4] = baseReg+1;
		ayData.txBuffer[6] = baseReg+1;
		ayData.txCount = 5;
	}
	ayData.timerVal = *(uint16_t*)current;

	iox->sendData(prepBuffer, 7);
    TIM_TypeDef * tim;
	if (baseReg == 0) {
		tim = TIM3;
	} else if (baseReg == 2) {
		tim = TIM4;
	} else if (baseReg == 4) {
		tim = TIM7;
	}
    CLEAR_BIT(tim->SR, TIM_FLAG_UPDATE);
    CLEAR_BIT(tim->SR, TIM_FLAG_UPDATE);
	tim->DIER |= TIM_IT_UPDATE;
    xSemaphoreTake(xIOXSemaphore, portMAX_DELAY);
}





void Player::resetParsers() {
    parsers.clear();
    chipV.clear();
    delete module;
    iox->reset();
}

void PlayerTask(__attribute__((unused)) void *pvParameters) {
    Player * player = new Player();
    player->run();
}


extern "C" {

// AY-3-8913 Chan A counter
void TIM3_IRQHandler() {
    CLEAR_BIT(TIM3->SR, TIM_FLAG_UPDATE);
	TIM3->DIER &= ~TIM_IT_UPDATE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
	   TIM3->ARR = ayData.timerVal - 1;
	   IoExpander::sendDataPolling(ayData.txBuffer, ayData.txCount);
	   if (xHigherPriorityTaskWoken != pdFALSE) {
		   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   }
   }
}

// AY-3-8913 Chan A counter
void TIM4_IRQHandler() {
    CLEAR_BIT(TIM4->SR, TIM_FLAG_UPDATE);
	TIM4->DIER &= ~TIM_IT_UPDATE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
	   TIM4->ARR = ayData.timerVal - 1;
	   IoExpander::sendDataPolling(ayData.txBuffer, ayData.txCount);
	   if (xHigherPriorityTaskWoken != pdFALSE) {
		   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   }
   }
}

// AY-3-8913 Chan A counter
void TIM7_IRQHandler() {
    CLEAR_BIT(TIM7->SR, TIM_FLAG_UPDATE);
	TIM7->DIER &= ~TIM_IT_UPDATE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
	   TIM7->ARR = ayData.timerVal - 1;
	   IoExpander::sendDataPolling(ayData.txBuffer, ayData.txCount);
	   if (xHigherPriorityTaskWoken != pdFALSE) {
		   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   }
   }
}
} // Extern "C"


