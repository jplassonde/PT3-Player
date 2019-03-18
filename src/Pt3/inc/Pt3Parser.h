#pragma once

#include <cstdio>
#include "Channel.h"

typedef struct SOUNDCHIP_T {
	uint8_t chanFreqAL;
	uint8_t chanFreqAH;
	uint8_t chanFreqBL;
	uint8_t chanFreqBH;
	uint8_t chanFreqCL;
	uint8_t chanFreqCH;
	uint8_t noiseFreq;
	uint8_t mixer;
	uint8_t volA;
	uint8_t volB;
	uint8_t volC;
	uint8_t envL;
	uint8_t envH;
	uint8_t envShape;
} SOUNDCHIP_T;

class Pt3Parser {
public:
	Pt3Parser(SOUNDCHIP_T * const chip, const uint8_t * module, bool loop);
	virtual ~Pt3Parser();
	bool processTick();

private:
	uint8_t parseLine(Channel * channel);
	bool advancePattern();
	void setEnvelopeSlide(const uint8_t * const data);
	void initEnvelope(uint8_t shape, uint8_t envH, uint8_t envL, Channel * channel);
	void processEnvSlide();
	SOUNDCHIP_T * const soundchip;
	const uint8_t * const module;

// Determine the number of tick (20ms) between line parsing
	uint8_t speed;
	uint8_t tick;

// Set song to loop or exit when end is reached
	bool loop;

// Pointer to useful spots in the .pt3
	const uint16_t * const pTable;
	const uint16_t * const ornTable;
	const uint16_t * const sampleTable;
	const uint8_t * const patternOrder;

//Pattern index
	uint8_t currentPattern;

// Channel instances
	Channel * channelA;
	Channel * channelB;
	Channel * channelC;

// Chip Global base values
	uint8_t baseNoise;
	uint16_t baseEnvelope;

//  Envelope Slide Parameters // to be moved when figured out.
	bool envSlideEnabled;
	uint16_t envSlide;
	uint8_t envDelay;
	uint8_t envTick;
	uint16_t envAcc;
};
