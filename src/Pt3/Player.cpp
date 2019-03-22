#include "Player.h"
#include <cstring>
#include "Project.h"
#include "module.h"

extern volatile uint8_t msFlag;
extern QueueHandle_t xPlayerCmdQueue;
extern SemaphoreHandle_t xPlayerTickSema;

Player::Player() {
	iox = new IoExpander();
	txArr[0] = IOA;
	playing = false;
	module = nullptr;
}

Player::~Player() {
	delete iox;
	delete module;
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
		parsers.clear();
		chipV.clear();
		delete module;

		module = pq->moduleAddr;

		initParser(AY30, module);

		if (strncmp((const char *)&module[pq->size-4], "02TS", 4) == 0) {
			uint16_t secondModIndex = *(uint16_t *)&module[pq->size-12];
			initParser(AY13, &module[secondModIndex]);
		}
		playing = true;
	}
}

void Player::initParser(uint8_t chipMask, uint8_t * modAddress) {
	const uint8_t lastStateInit[13] = {0xA5};
	std::unique_ptr<CHIPSTATE_T> chip((CHIPSTATE_T*)pvPortMalloc(sizeof(CHIPSTATE_T)));
	memcpy(&chip->last, &lastStateInit, 13);
	chip->current.mask = chipMask;


	std::unique_ptr<Pt3Parser> parser(new Pt3Parser(&(chip->current), modAddress, true));

	chipV.push_back(std::move(chip));
	parsers.push_back(std::move(parser));
}

void Player::play() {
	uint8_t length = 1;

	for (auto &i : parsers) {
		i->processTick();
	}

	for (auto &i : chipV) {
		length = buildArray(length, (uint8_t *)&i->current, (uint8_t *)&i->last, i->current.mask);
		memcpy(&i->last, &i->current, sizeof(SOUNDCHIP_T));
	}
	txArr[length++] = AY30 | AY13;

	xSemaphoreTake(xPlayTickSema, portMAX_DELAY);

	if (length > 5) {
		iox->sendData(txArr, length);

	}
}

uint8_t Player::buildArray(uint8_t index, uint8_t * current, uint8_t * previous, uint8_t mask) {
	uint8_t latchAddr = mask | BDIR | BC1;
	uint8_t writeData = mask | BDIR;
    uint8_t busInactive = mask;

    // Send updated data using one long transfer with bus transactions, IO expender will alternate between port
    // value at index 15 indicate a reset of the envelope period.
    for (int i = 0; i <= 13; i++) {
    	if  (current[i] != previous[i] || (i == 13 && current[15] == 1)) {
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
    current[15] = 0;
    return index;
}

void PlayerTask(__attribute__((unused)) void *pvParameters) {
	Player * player = new Player();
	player->run();
}
