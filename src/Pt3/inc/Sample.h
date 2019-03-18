#pragma once

#include <cstdint>

typedef struct SAMPLE_DATA_T {
	uint8_t mask;
	uint8_t volume;
	uint8_t noiseOffset;
	uint8_t envelopeOffset;
	uint16_t toneShift;
} SAMPLE_DATA_T;

class Sample {
public:
	Sample(const uint8_t * base);
	virtual ~Sample();
	void processSample(SAMPLE_DATA_T * sampleData);
	void initSample(const uint8_t * base);
	void setOffset(uint8_t offset);
	void reset();
private:
	const uint8_t * sampleBase;
	uint8_t end;
	uint8_t loop;
	uint8_t idx;

	uint16_t accTone;
	uint8_t accNoise;
	uint8_t accEnv;
	int8_t volModif;
};
