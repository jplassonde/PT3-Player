#include "Idle.h"
#include "Browser.h"
#include "project.h"
#include "listitem750x50.h"
#include <cstring>
#include <algorithm>
#include "PlayerQueue.h"
#include "PlayerDisplay.h"

extern QueueHandle_t xPlayerCmdQueue;

constexpr uint8_t ITEM_HEIGHT = 50;


void (ScreenElement::* eventPtr[3]) (TOUCH_EVENT_T) = {&ScreenElement::press, &ScreenElement::liftOff, &ScreenElement::contact};
bool compareItems(std::unique_ptr<ListElement> &l1, std::unique_ptr<ListElement> &l2);

TCHAR * Browser::path = nullptr;

Browser::Browser(MainEngine * mainEngine) : mainEngine(mainEngine), offset(0) {
	if (path == nullptr) {
		path = (TCHAR *)pvPortMalloc(7*sizeof(TCHAR));
		strcpy(path, "/files");
	}

	auto scrollerCallback = std::bind(&Browser::scrollerCB, this,std::placeholders::_1);
	scrollbar = std::make_unique<Scrollbar>(750,50, scrollerCallback);

	buildDirList();
	markedForDeletion = false; // This here is the perfect example of a symptom of bad design
}

Browser::~Browser() {
}

void Browser::processTouch(TOUCH_EVENT_T touchData) {
	// Process action on element first
	for (auto &i : listV) {
		if (i->isInside(touchData.xPosition, touchData.yPosition)) {
			(*i.*(eventPtr[touchData.touchEvent]))(touchData);
			if (markedForDeletion) { // This has to be the worse crime I've ever commited
				delete this;
				return;
			}
			break;
		}
	}

	if (scrollbar->isInside(touchData.xPosition, touchData.yPosition)) {
		(*scrollbar.*(eventPtr[touchData.touchEvent]))(touchData);
	}


	// Then adjust position on swipe up-down
	if (touchData.xPosition < 700) { // leave 100 px right, not to interfer with scrollbar
		if (touchData.gesture == SWIPE_UP) {
			swipeUp(touchData.gestureMagnitude);
		} else if (touchData.gesture == SWIPE_DOWN) {
			swipeDown(touchData.gestureMagnitude);
		}
	}
}

void Browser::drawScreen() {
	for (auto &i : listV) {
		i->draw();
	}
	if (listV.size() < 9) {
		padScreen(listV.size());
	}

	scrollbar->draw();
	mainEngine->addScanlines();
}

void Browser::buildDirList() {
	auto cdPtr = std::bind(&Browser::changeDirectory, this, std::placeholders::_1, std::placeholders::_2);
	auto playPtr = std::bind(&Browser::play, this, std::placeholders::_1, std::placeholders::_2);
	auto previousDir = std::bind(&Browser::previousDir, this,std::placeholders::_1, std::placeholders::_2);

	FRESULT res;
	f_mount(&fs, "", 0);
	res = f_opendir(&dp, path);
	if (res != FR_OK) {
		while(1);
	}
	while(1) {
		res = f_readdir(&dp, &fno);
		if (res != FR_OK || fno.fname[0] == 0) {
			break;
		}
		std::unique_ptr<ListElement>listElement(new ListElement(&fno, (fno.fattrib & AM_DIR ? cdPtr : playPtr)));
		listV.push_back(std::move(listElement));
	}

	// Hax for previous dir....
	if (strlen(path) > 6) {
		strcpy(fno.fname,"...");
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

void Browser::changeDirectory(TCHAR * name, FSIZE_t size) {
	offset = 0;
	TCHAR * newPath = (TCHAR*)pvPortMalloc((strlen(path)+strlen(name)+2)*sizeof(TCHAR));
	strcpy(newPath, path);
	strcat(newPath, "/");
	strcat(newPath, name);
	vPortFree(path);
	path = newPath;
	listV.clear();
	buildDirList();
}

void Browser::play(TCHAR * name, FSIZE_t size) {
	FIL fp;
	UINT numRead;
	uint8_t * file = (uint8_t *)pvPortMalloc(size*sizeof(TCHAR));
	f_mount(&fs, "", 0);
	f_chdir(path);
	f_open(&fp, name, FA_READ);
	f_read(&fp, (void*)file, size, &numRead);
	f_close(&fp);
	f_mount(0, "", 0);

	PLAYER_QUEUE_T pq;
	pq.cmd = PLAY;
	pq.moduleAddr = file;
	xQueueSend(xPlayerCmdQueue, (void *)&pq, portMAX_DELAY);

	// Mmm... The sweet smell of overcooked pastas.
	markedForDeletion = true;
	PlayerDisplay * pd = new PlayerDisplay((uint8_t *)name, &file[30], &file[66], mainEngine);
	mainEngine->switchState(pd);
}

void Browser::previousDir(TCHAR * name, FSIZE_t size) {
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
		buildDirList();
	}
}

bool Browser::scrollerCB(float percent) {
	if (listV.size() >= 9) {
		uint32_t maxOffset = listV.size()*ITEM_HEIGHT - 430;
		offset = maxOffset*percent/100;
		setItemPos();
		return true;
	}
	return false;
}

void Browser::setItemPos() {
	int position = 50;
	for ( auto &i : listV ) {
		i->setY(position-offset);
		position+=50;
	}

	uint32_t maxOffset = listV.size()*ITEM_HEIGHT - 430;
	scrollbar->setCursor((float)offset*350/maxOffset);

}

void Browser::padScreen(uint8_t padStart) {
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

void Browser::swipeDown(uint16_t mag) {
	if (offset - mag < 0) {
		offset = 0;
	} else {
		offset-=mag;
	}
	setItemPos();
}

void Browser::swipeUp(uint16_t mag) {
	uint32_t maxOffset = (listV.size() <= 8 ? 0 : listV.size()*ITEM_HEIGHT - 430);
	offset += mag;
	if (offset > maxOffset) {
		offset = maxOffset;
	}
	setItemPos();
}

// Global compare function, for sort
bool compareItems(std::unique_ptr<ListElement> &l1, std::unique_ptr<ListElement> &l2) {
	// Place directories first
	if (l1->getType() != l2->getType()) {
		return l1->getType() < l2->getType();
	}
	// Then sort alphabetically, case insensitive

	for (uint8_t index = 0; index < strlen(l1->getName()) + 1; index++) {
		if (toupper(l1->getName()[index]) != toupper(l2->getName()[index])) {
			return toupper(l1->getName()[index]) < toupper(l2->getName()[index]);
		}
	}
	return true; // in case of equal name(won't happen). Null char of shorter str will take care of it.*/
}
