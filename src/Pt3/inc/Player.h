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
#include <vector>
#include <memory>

typedef struct CHIPSTATE_T {
	SOUNDCHIP_T last;
	SOUNDCHIP_T current;
} CHIPSTATE_T;

class Player {
public:
	Player();
	virtual ~Player();
	void run();
private:
	uint8_t play();
	void processQueue(PLAYER_QUEUE_T * pq);
	void initParser(uint8_t chipMask, uint8_t * modAddress);
	uint8_t buildArray(uint16_t startPos, uint8_t * chip, uint8_t * prevVals, const uint8_t chipMask);
	void loadNext();
	void loadModule();
	void resetParsers();
	void updateAY13Tone(uint8_t * current, uint8_t * previous);
	void updateAY13Chan(uint8_t * current, uint8_t * previous, uint8_t baseReg);
	void updateLeds();
	std::vector<std::unique_ptr<CHIPSTATE_T>> chipV;
	std::vector<std::unique_ptr<Pt3Parser>> parsers;

	FsFolder * folder;
	IoExpander * iox;
	uint8_t * module;
	uint8_t txArr[512];
	bool playing;
	uint32_t loopTime;

};

#endif /* PT3_PLAYER_H_ */
