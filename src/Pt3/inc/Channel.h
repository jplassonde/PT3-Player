#ifndef PT3_CHANNEL_H_
#define PT3_CHANNEL_H_

#include <cstdint>
#include "Sample.h"
#include "Ornament.h"

typedef struct CHANNEL_T {
	uint8_t chanId;
	uint8_t * freqL;
	uint8_t * freqH;
	uint8_t * noiseFreq;
	uint8_t * mixer;
	uint8_t * vol;
	uint8_t * envL;
	uint8_t * envH;
	uint8_t * envShape;
	uint8_t * speed;
} CHANNEL_T;

class Channel {

public:
	enum channelId : uint8_t {chanA = 0x00, chanB = 0x01, chanC = 0x02};
	Channel(channelId chan, uint16_t startingPos, const uint8_t * sampBase, const uint8_t * ornBase, const uint16_t * ft);
	virtual ~Channel();
	bool hasLine();

	uint8_t channel;
	uint16_t pos;
	void setGliss(const uint8_t * const args);
	void setPortamento(const uint8_t * const args);
	void savePortParams();
	void setSampleOffset(uint8_t offset);
	void setOrnamentOffset(uint8_t offset);
	void setVibrato(const uint8_t * const args);
	void setSample(const uint8_t * sampleBase);
	void setOrnament(const uint8_t * ornamentBase);
	void setTone(uint8_t toneIdx);
	void disableEnvelope();
	void enableEnvelope();
	void setBlanks(uint8_t blank);
	void setBaseVol(uint8_t vol);
	bool isBlank();
	bool isPlaying();
	void mute();
	void unmute();
	void processChanTick(uint8_t * freqH, uint8_t * freqL,
						 uint8_t * mixer, uint8_t * vol,
						 uint8_t * noise, uint16_t * env, uint8_t chipId);
	uint8_t baseToneIdx;
private:
	void (Channel::*effectFunct)(SAMPLE_DATA_T * sd);
	void processGliss(SAMPLE_DATA_T * sd);
	void processPortamento(SAMPLE_DATA_T * sd);
	void processVibrato(SAMPLE_DATA_T * sd);
	bool playing;
	const uint16_t * freqTable;
// Tone
	int16_t accTone;

// Volume
	uint8_t volume;
	uint8_t masterEnvelope;

// Sample and Ornaments
	Sample * sample;
	Ornament * ornament;
	uint8_t ornamentTick;

// Delay beween line parsing
	uint8_t inactiveLines;
	uint8_t inactCounter;

// Effect
	const uint8_t * effectArg;
	uint8_t effectTick;
	// temp hax to fix portamento
	uint8_t tempToneIdx;
	int16_t tempAccTone;
	int16_t step;
};



#endif /* PT3_CHANNEL_H_ */
