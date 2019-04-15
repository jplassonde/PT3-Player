
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


static bool compareItems(std::unique_ptr<ListElement> &l1, std::unique_ptr<ListElement> &l2);

TrackList::TrackList(MainEngine * me) : mainEngine(me) {
	yPos = 50;
	xPos = 0;
	auto scrollerCallback = std::bind(&TrackList::scrollerCB, this,std::placeholders::_1);
	scrollbar = std::make_unique<Scrollbar>(750,50, scrollerCallback);

	path = (TCHAR *)pvPortMalloc(7*sizeof(TCHAR));
	strcpy(path, "/files");

	buildList();
}

TrackList::~TrackList() {}

void TrackList::draw() {
	uint16_t firstElem = offset/ITEM_HEIGHT;
	uint16_t lastElem = (offset+LIST_HEIGHT-1)/ITEM_HEIGHT;
	lastElem = listV.size() >= 9 ? lastElem : listV.size() - 1 ;

	for (auto it = listV.begin()+firstElem; it <= listV.begin()+lastElem; it++) {
		(*it)->draw();
	}

	if (listV.size() < 9) {
		padScreen(listV.size());
	}

	scrollbar->draw();
}

void TrackList::press(TOUCH_EVENT_T touchEvent) {
	if (touchEvent.xPosition <= 700) {
		uint16_t idx = (touchEvent.yPosition - yPos + offset)/50;
		if (idx < listV.size()) {
			listV[idx]->press(touchEvent);
		}
	} else {
		scrollbar->press(touchEvent);
	}
}

void TrackList::contact(TOUCH_EVENT_T touchEvent) {
	if (touchEvent.xPosition <= 700) {
		uint16_t firstElem = offset/ITEM_HEIGHT;
		uint16_t lastElem = (offset+LIST_HEIGHT-1)/ITEM_HEIGHT;
		lastElem = listV.size() >= 9 ? lastElem : listV.size() - 1;

		for (auto it = listV.begin()+firstElem; it <= listV.begin()+lastElem; it++) {
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
		uint16_t idx = (touchEvent.yPosition - yPos + offset)/50;
		if (idx < listV.size()) {
			listV[idx]->liftOff(touchEvent);
		}
	} else {
		scrollbar->liftOff(touchEvent);
	}
}

bool TrackList::isInside(uint16_t x, uint16_t y) {
	return (y >= yPos);
}

bool TrackList::scrollerCB(float percent) {
	if (listV.size() >= 9) {
		uint32_t maxOffset = listV.size()*ITEM_HEIGHT - LIST_HEIGHT;
		offset = maxOffset*percent/100;
		setItemPos();
		return true;
	}
	return false;
}

void TrackList::swipeUp(uint16_t mag) {
	uint32_t maxOffset = (listV.size() <= 8 ? 0 : listV.size()*ITEM_HEIGHT - 430);
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
		offset-=mag;
	}
	setItemPos();
}

void TrackList::buildList() {
	auto cdPtr = std::bind(&TrackList::changeDirectory, this, std::placeholders::_1, std::placeholders::_2);
	auto playPtr = std::bind(&TrackList::play, this, std::placeholders::_1, std::placeholders::_2);
	auto previousDir = std::bind(&TrackList::previousDir, this,std::placeholders::_1, std::placeholders::_2);

	FATFS fs;
	FRESULT res;
	DIR dp;
	FILINFO fno;

	f_mount(&fs, "", 0);
	res = f_opendir(&dp, path);

	while(1) {
		res = f_readdir(&dp, &fno);
		if (res != FR_OK || fno.fname[0] == 0) {
			break;
		}
		std::unique_ptr<ListElement>listElement(new ListElement(&fno, (fno.fattrib & AM_DIR ? cdPtr : playPtr)));
		listV.push_back(std::move(listElement));
	}


	if (strlen(path) > 6) {
		strcpy(fno.fname,"..");
		fno.fattrib = AM_DIR;
		std::unique_ptr<ListElement>listElement(new ListElement(&fno,  previousDir));
		listV.push_back(std::move(listElement));
	}

	f_closedir(&dp);
	f_mount(0, "", 0);

	std::sort(listV.begin(), listV.end(), compareItems);
	offset = 0;
	setItemPos();
}

void TrackList::setItemPos() {
	int position = yPos;
	for ( auto &i : listV ) {
		i->setY(position-offset);
		position+=50;
	}
	uint32_t maxOffset = listV.size()*ITEM_HEIGHT - 430;
	scrollbar->setCursor((float)offset*350/maxOffset);
}

void TrackList::padScreen(uint8_t padStart) {
	uint8_t height = ITEM_HEIGHT;
	for (uint8_t i = padStart; i < 9; i++) {
		if (i == 8) {
			height = 30;
		}
		uint32_t destination = Screen::getBackBufferAddr()+800*(i+1)*50*4;
		uint32_t source = (uint32_t)listitem;
		hdma2d.Init.Mode         = DMA2D_M2M_PFC;
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

void TrackList::play(TCHAR * name, FSIZE_t size) {
	FATFS fs;
	FIL fp;
	UINT numRead;
	uint8_t * file = (uint8_t *)pvPortMalloc(size*sizeof(TCHAR));

	f_mount(&fs, "", 0);
	f_chdir(path);
	f_open(&fp, name, FA_READ);
	f_read(&fp, (void*)file, size, &numRead);
	f_close(&fp);
	f_mount(0, "", 0);

	mainEngine->play((uint8_t *)name, file, size);

}

void TrackList::changeDirectory(TCHAR * name, FSIZE_t size) {
	offset = 0;
	TCHAR * newPath = (TCHAR*)pvPortMalloc((strlen(path)+strlen(name)+2)*sizeof(TCHAR));
	strcpy(newPath, path);
	strcat(newPath, "/");
	strcat(newPath, name);
	vPortFree(path);
	path = newPath;
	listV.clear();
	buildList();
}

void TrackList::previousDir(TCHAR * name, FSIZE_t size) {
	uint8_t lastSlash = 0;
	int i = 6; // use /files as root
	while(path[i]) {
		if (path[i] == '/') {
			lastSlash = i;
		}
		i++;
	}
	if (lastSlash != 0) {
		path[lastSlash] = 0;
		listV.clear();
		buildList();
	}
}

// Global compare function, for sort
bool compareItems(std::unique_ptr<ListElement> &l1, std::unique_ptr<ListElement> &l2) {
	// Place directories first
	if (l1->getType() != l2->getType()) {
		return l1->getType() < l2->getType();
	}
	// Then sort alphabetically, case insensitive
	for (uint8_t index = 0; index <= strlen(l1->getName()); index++) {
		if (toupper(l1->getName()[index]) != toupper(l2->getName()[index])) {
			return toupper(l1->getName()[index]) < toupper(l2->getName()[index]);
		}
	}
	return true; // in case of equal name(won't happen). Null char of shorter str will take care of it.*/
}
