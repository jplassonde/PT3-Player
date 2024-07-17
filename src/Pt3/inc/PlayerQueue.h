#pragma once

#define PLAY 0x00
#define PAUSE 0x01
#define FF 0x02
#define STOP 0x03

#include "FsFolder.h"
#include <memory>

typedef struct PLAYER_QUEUE_T {
	uint8_t cmd;

	union {
		FsFolder * folder;
		uint32_t pos;
	};
} PLAYER_QUEUE_T;
