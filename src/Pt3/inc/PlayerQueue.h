#pragma once

#define PLAY 0x00
#define PAUSE 0x01
#define STOP 0x02

typedef struct PLAYER_QUEUE_T {
	uint8_t cmd;
	uint8_t * moduleAddr;
} PLAYER_QUEUE_T;
