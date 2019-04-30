#pragma once

#include "project.h"
#include "diskio.h"


void sdInit();

class SDCard {
public:
	SDCard();
	virtual ~SDCard();
	DSTATUS getStatus();
	DRESULT sdRead(uint8_t * buff, uint32_t firstSector, uint32_t sectorCount);
};
