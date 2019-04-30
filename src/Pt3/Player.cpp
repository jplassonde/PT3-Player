#include "Player.h"
#include <cstring>
#include "Project.h"
#include "TrackInfos.h"
#include "IEvent.h"
#include "TrackTime.h"

extern volatile uint8_t msFlag;
extern QueueHandle_t xPlayerCmdQueue;
extern QueueHandle_t xIeQueue;
extern SemaphoreHandle_t xPlayerTickSema;

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
            play();
        } else if (xQueueReceive(xPlayerCmdQueue, (void*)&pq, portMAX_DELAY) == pdTRUE) { // Hang until queue receive if not playing
            processQueue(&pq);
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

        default:
            while(1); //something is broken, debug.
    }
}

void Player::initParser(uint8_t chipMask, uint8_t * modAddress) {
    const uint8_t lastStateInit[13] = { 0xA5 };
    std::unique_ptr<CHIPSTATE_T> chip((CHIPSTATE_T*)pvPortMalloc(sizeof(CHIPSTATE_T)));
    memcpy(&chip->last, &lastStateInit, 13);
    chip->current.mask = chipMask;

    std::unique_ptr<Pt3Parser> parser(new Pt3Parser(&(chip->current), modAddress));

    chipV.push_back(std::move(chip));
    parsers.push_back(std::move(parser));
}

void Player::play() {
    uint8_t length = 1; // Total message length, first byte is IoX address.
    bool notOver;

    for (auto &i : parsers) {
        notOver = i->processTick();
    }

    if (notOver == false) {
        loadNext();
        for (auto &i : parsers) {
            i->processTick();
        }
    }

    for (auto &i : chipV) {
        length = buildArray(length, (uint8_t *)&i->current, (uint8_t *)&i->last,
                i->current.mask);
        memcpy(&i->last, &i->current, sizeof(SOUNDCHIP_T));
    }

    txArr[length++] = AY30 | AY13;

    xSemaphoreTake(xPlayTickSema, portMAX_DELAY);
    trackTime->incCurrent();

    if (length > 5) { // Don't bother sending if there is no register update
        iox->sendData(txArr, length);
    }
}

void Player::loadNext() {
    folder->advanceFile();
    loadModule();
}

void Player::loadModule() {
    parsers.clear();
    chipV.clear();
    delete module;
    iox->reset();

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

uint8_t Player::buildArray(uint8_t index, uint8_t * current, uint8_t * previous, uint8_t mask) {
    uint8_t latchAddr = mask | BDIR | BC1;
    uint8_t writeData = mask | BDIR;
    uint8_t busInactive = mask | AY30;

    // Send updated data using one long transfer with bus transactions, IO expander will alternate between ports
    // value at index 15 indicate a reset of the envelope period -> needs to be updated even if it has the same value
    for (int i = 0; i <= 13; i++) {
        if (current[i] != previous[i] || (i == 13 && current[15] == 1)) {
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
