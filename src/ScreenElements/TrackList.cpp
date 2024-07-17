#include "TrackList.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>

#include "Screen.h"
#include "PlayerQueue.h"
#include "listitem750x50.h"

constexpr uint8_t ITEM_HEIGHT = 50;
constexpr uint16_t LIST_HEIGHT = 430;

extern QueueHandle_t xPlayerCmdQueue;
extern QueueHandle_t xMp3PlayerCmdQueue;

TrackList::TrackList(MainEngine * me) :
        mainEngine(me), folder() {
    yPos = 50;
    xPos = 0;
    auto scrollerCallback = std::bind(&TrackList::scrollerCB, this, std::placeholders::_1);
    scrollbar = std::make_unique<Scrollbar>(750, 50, scrollerCallback);

    buildList();
}

TrackList::~TrackList() {
}

void TrackList::draw() {
    uint16_t firstElem = offset / ITEM_HEIGHT;
    uint16_t lastElem = (offset + LIST_HEIGHT - 1) / ITEM_HEIGHT;
    lastElem = listV.size() >= 9 ? lastElem : listV.size() - 1;

    for (auto it = listV.begin() + firstElem; it <= listV.begin() + lastElem; it++) {
        (*it)->draw();
    }

    if (listV.size() < 9) {
        padScreen(listV.size());
    }

    scrollbar->draw();
}

void TrackList::press(TOUCH_EVENT_T touchEvent) {
    if (touchEvent.xPosition <= 700) {
        uint16_t idx = (touchEvent.yPosition - yPos + offset) / 50;
        if (idx < listV.size())
            listV[idx]->press(touchEvent);
    } else {
        scrollbar->press(touchEvent);
    }
}

void TrackList::contact(TOUCH_EVENT_T touchEvent) {
    if (touchEvent.xPosition <= 700) {
        uint16_t firstElem = offset / ITEM_HEIGHT;
        uint16_t lastElem = (offset + LIST_HEIGHT - 1) / ITEM_HEIGHT;
        lastElem = listV.size() >= 9 ? lastElem : listV.size() - 1;

        for (auto it = listV.begin() + firstElem; it <= listV.begin() + lastElem; it++) {
            if ((*it)->isInside(touchEvent.xPosition, touchEvent.yPosition)) {
                (*it)->contact(touchEvent);
            }
        }

        if (touchEvent.gesture == SWIPE_UP) {
            swipeUp(touchEvent.gestureMagnitude);
        } else if (touchEvent.gesture == SWIPE_DOWN) {
            swipeDown(touchEvent.gestureMagnitude);
        }

    } else {
        scrollbar->contact(touchEvent);
    }
}

void TrackList::liftOff(TOUCH_EVENT_T touchEvent) {
    if (touchEvent.xPosition <= 700) {
        uint16_t idx = (touchEvent.yPosition - yPos + offset) / 50;
        if (idx < listV.size()) {
            listV[idx]->liftOff(touchEvent);
        }
    }
    scrollbar->liftOff(touchEvent);
}

bool TrackList::isInside(uint16_t x, uint16_t y) {
    return (y >= yPos);
}

bool TrackList::scrollerCB(float percent) {
    if (listV.size() >= 9) {
        uint32_t maxOffset = listV.size() * ITEM_HEIGHT - LIST_HEIGHT;
        offset = maxOffset * percent / 100;
        setItemPos();
        return true;
    }
    return false;
}

void TrackList::swipeUp(uint16_t mag) {
    uint32_t maxOffset = (listV.size() <= 8 ? 0 : listV.size() * ITEM_HEIGHT - 430);
    offset += mag;
    if (offset > maxOffset) {
        offset = maxOffset;
    }
    setItemPos();
}

void TrackList::swipeDown(uint16_t mag) {
    if (offset - mag < 0) {
        offset = 0;
    } else {
        offset -= mag;
    }
    setItemPos();
}

void TrackList::buildList() {
    auto cdPtr = std::bind(&TrackList::changeDirectory, this, std::placeholders::_1);
    auto playPtr = std::bind(&TrackList::play, this, std::placeholders::_1);
    auto previousDir = std::bind(&TrackList::previousDir, this, std::placeholders::_1);

    listV.clear();

    const std::shared_ptr<std::vector<std::shared_ptr<FileInfos>>>files = folder.getFiles();
    const std::shared_ptr<std::vector<std::shared_ptr<TCHAR>>>subdirs = folder.getDirs();

    if (strlen(folder.getPath()) > 6) { // if not on root (/files) folder, add a previous-dir link
        std::shared_ptr<TCHAR> prevDir = std::shared_ptr<TCHAR>((TCHAR*)pvPortMalloc(3 * sizeof(TCHAR)));
        strcpy(prevDir.get(), "..");
        std::unique_ptr<ListElement> listElement(new ListElement(prevDir, previousDir));
        listV.push_back(std::move(listElement));
    }

    for (auto &i : *subdirs.get()) {
        std::unique_ptr<ListElement> listElement(new ListElement(i, cdPtr));
        listV.push_back(std::move(listElement));
    }

    for (auto &i : *files.get()) {
        std::unique_ptr<ListElement> listElement(new ListElement(i, playPtr));
        listV.push_back(std::move(listElement));
    }

    offset = 0;
    setItemPos();
}

void TrackList::setItemPos() {
    int position = yPos;
    for (auto &i : listV) {
        i->setY(position - offset);
        position += 50;
    }
    uint32_t maxOffset = listV.size() * ITEM_HEIGHT - 430;
    scrollbar->setCursor((float)offset * 350 / maxOffset);
}

void TrackList::padScreen(uint8_t padStart) {
    uint8_t height = ITEM_HEIGHT;
    for (uint8_t i = padStart; i < 9; i++) {
        if (i == 8)
            height = 30;

        uint32_t destination = Screen::getBackBufferAddr() + 800 * (i + 1) * 50 * 4;
        uint32_t source = (uint32_t)listitem;
        hdma2d.Init.Mode = DMA2D_M2M_PFC;
        hdma2d.Init.OutputOffset = 50;
        hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
        hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputOffset = 0;
        HAL_DMA2D_Init(&hdma2d);
        HAL_DMA2D_ConfigLayer(&hdma2d, 1);

        HAL_DMA2D_Start_IT(&hdma2d, source, destination, 750, height);
        xSemaphoreTake(xDma2dSemaphore, portMAX_DELAY);
    }
}

void TrackList::play(std::shared_ptr<TCHAR> name) {
    folder.setActiveFile(name);
    PLAYER_QUEUE_T pq;
    pq.cmd = PLAY;
    pq.folder = new FsFolder(folder);

    char * extension = strrchr(name.get(), '.') + 1;

    if (strcmp(extension, "mp3") == 0) {
    	xQueueSend(xMp3PlayerCmdQueue, (void * )&pq, portMAX_DELAY);
    	pq = {0};
    	pq.cmd = STOP;

    } else if (strcmp(extension, "pt3") == 0) {
    	xQueueSend(xPlayerCmdQueue, (void * )&pq, portMAX_DELAY);
    }
}

void TrackList::changeDirectory(std::shared_ptr<TCHAR> name) {
    folder.enterDirectory(name.get());
    buildList();
}

void TrackList::previousDir(__attribute__((unused))  std::shared_ptr<TCHAR> name) {
    folder.setDirPrevious();
    buildList();

}
