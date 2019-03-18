#include "Player.h"
#include <cstring>
#include "Project.h"
#include "module.h"

extern volatile uint8_t msFlag;
extern QueueHandle_t xPlayerCmdQueue;
extern SemaphoreHandle_t xPlayerTickSema;

bool envelopeUpdated; // unused
bool envReset;        // hack

Player::Player() : lastState({0xA5}),
				   soundchip({0x00}) {

	iox = new IoExpander();
	txArr[0] = IOA;
	playing = false;
	parser = nullptr;
	envelopeUpdated = true;
	envReset = false;
}

Player::~Player() {
	delete parser;
	delete iox;
}

void Player::run() {
	PLAYER_QUEUE_T pq;

	while(1) {
		if (xQueueReceive(xPlayerCmdQueue, (void*)&pq, 0) == pdTRUE) { // check queue between each tick. Will be useful
			processQueue(&pq);										   // when other commands (pause, etc...) will be added
		}

		if (playing) {
			play();
		} else { // Hang until queue receive if not playing
			if (xQueueReceive(xPlayerCmdQueue, (void*)&pq, portMAX_DELAY) == pdTRUE) {
				processQueue(&pq);
			}
		}
	} // end while(1);
} // end run

void Player::processQueue(PLAYER_QUEUE_T * pq) {
	if (pq->cmd == PLAY) {
		if (parser != nullptr) {
			delete parser;
		}
		parser = new Pt3Parser(&soundchip, pq->moduleAddr, true);
		playing = true;
	}
}

void Player::play() {
	parser->processTick();

	uint8_t length = buildArray(1, (uint8_t *)&soundchip, (uint8_t *)&lastState, AY30);

	xSemaphoreTake(xPlayTickSema, portMAX_DELAY);

	if (length > 5) {
		iox->sendData(txArr, length);
		memcpy(&lastState, &soundchip, sizeof(SOUNDCHIP_T));
	}
}

uint8_t Player::buildArray(uint8_t index, uint8_t * chip, uint8_t * prevVals, const uint8_t chipMask) {
	uint8_t latchAddr = chipMask | BDIR | BC1;
	uint8_t writeData = chipMask | BDIR;
    uint8_t busInactive = chipMask;

    // Send updated data using one long transfer with bus transactions, IO expender will alternate between port
    //+ Small hack to reset envelope event though it didnt "change" value.
    for (int i = 0; i <= 13; i++) {
    	if (chip[i] != prevVals[i] || (i == 13 && envReset)) {
       		txArr[index++] = busInactive;
    		txArr[index++] = i;
    		txArr[index++] = latchAddr;
    		txArr[index++] = i;
    		txArr[index++] = busInactive;
    		txArr[index++] = chip[i];
    		txArr[index++] = writeData;
    		txArr[index++] = chip[i];
    	}
    }

    txArr[index++] = busInactive;
    envelopeUpdated = false;
    envReset = false;
    return index;
}

void PlayerTask(__attribute__((unused)) void *pvParameters) {
	Player * player = new Player();
	player->run();
}
