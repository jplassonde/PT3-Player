/*
 * Player.h
 *
 *  Created on: Feb 25, 2019
 *      Author: xbg
 */

#ifndef PT3_PLAYER_H_
#define PT3_PLAYER_H_

#include "Pt3Parser.h"
#include "IoExpander.h"
#include <cstdio>
#include "PlayerQueue.h"
class Player {
public:
	Player();
	virtual ~Player();
	void run();

private:
	void play();
	void processQueue(PLAYER_QUEUE_T * pq);
	Pt3Parser * parser;
	uint8_t buildArray(uint8_t startPos, uint8_t * chip, uint8_t * prevVals, const uint8_t chipMask);
	SOUNDCHIP_T lastState;
	SOUNDCHIP_T soundchip;
	IoExpander * iox;
	uint8_t txArr[208];
	bool playing;
};

#endif /* PT3_PLAYER_H_ */
