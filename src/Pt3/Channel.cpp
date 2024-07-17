#include "Channel.h"
#include <cmath>
#include <cstdlib>

// Volume table, base*sample volume combinations.
const uint8_t volTable[16][16]={
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
  {0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02},
  {0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03},
  {0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04},
  {0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05},
  {0x0,0x0,0x1,0x1,0x2,0x2,0x2,0x3,0x3,0x4,0x4,0x4,0x5,0x5,0x6,0x6},
  {0x0,0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7},
  {0x0,0x1,0x1,0x2,0x2,0x3,0x3,0x4,0x4,0x5,0x5,0x6,0x6,0x7,0x7,0x8},
  {0x0,0x1,0x1,0x2,0x2,0x3,0x4,0x4,0x5,0x5,0x6,0x7,0x7,0x8,0x8,0x9},
  {0x0,0x1,0x1,0x2,0x3,0x3,0x4,0x5,0x5,0x6,0x7,0x7,0x8,0x9,0x9,0xA},
  {0x0,0x1,0x1,0x2,0x3,0x4,0x4,0x5,0x6,0x7,0x7,0x8,0x9,0xA,0xA,0xB},
  {0x0,0x1,0x2,0x2,0x3,0x4,0x5,0x6,0x6,0x7,0x8,0x9,0xA,0xA,0xB,0xC},
  {0x0,0x1,0x2,0x3,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xA,0xB,0xC,0xD},
  {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE},
  {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF},
};


Channel::Channel(channelId chan, uint16_t startingPos, const uint8_t * sampBase, const uint8_t * ornBase, const uint16_t * ft) :
        channel(chan), pos(startingPos), freqTable(ft) {

	sample = new Sample(sampBase);
    ornament = new Ornament(ornBase);
    effectFunct = nullptr;
    inactiveLines = 0;
    inactCounter = 0;
    masterEnvelope = 1;
    volume = 0x0F;
    playing = false;
}

Channel::~Channel() {
    delete sample;
    delete ornament;
}

void Channel::setGliss(const uint8_t * const args) {
    effectFunct = &Channel::processGliss;
    effectArg = args;
    effectTick = 0;
}

void Channel::setPortamento(const uint8_t * const args) {
    int16_t toneDiff = freqTable[tempToneIdx] - freqTable[baseToneIdx];
    accTone = toneDiff + tempAccTone;
    step = *(int16_t*)&args[3];
    if (((accTone < 0) && (step < 0)) || ((accTone > 0) && (step > 0))) {
    	step = -step;
    }
    effectFunct = &Channel::processPortamento;
    effectArg = args;
    effectTick = 0;
}

void Channel::setSampleOffset(uint8_t offset) {
    sample->setOffset(offset);
}

void Channel::setOrnamentOffset(uint8_t offset) {
    ornament->setTickOffset(offset);
}

void Channel::setVibrato(const uint8_t * const args) {
    effectFunct = &Channel::processVibrato;
    effectArg = args;
    effectTick = 0;
}

void Channel::setSample(const uint8_t * sampleBase) {
    sample->initSample(sampleBase);
}

void Channel::setOrnament(const uint8_t * ornamentBase) {
    ornament->setOrnament(ornamentBase);
}

void Channel::setTone(uint8_t toneIdx) {
    effectFunct = nullptr;
    baseToneIdx = toneIdx;
    sample->reset();
    ornament->reset();
    playing = true;
    accTone = 0;
}

void Channel::disableEnvelope() {
    masterEnvelope = 1;
}

void Channel::enableEnvelope() {
    masterEnvelope = 0;
}
void Channel::setBlanks(uint8_t blank) {
    inactiveLines = blank;
}

void Channel::setBaseVol(uint8_t vol) {
    volume = vol;
}

void Channel::processGliss(SAMPLE_DATA_T * sd) {
    if (effectTick >= effectArg[0] && effectArg[0] != 0) {
        accTone += *(uint16_t*)&effectArg[1];
        effectTick = 0;
    }
    sd->toneShift += accTone;
    ++effectTick;
}


// Really need to investigate how the portamento are processed and clean this hack

void Channel::processPortamento(SAMPLE_DATA_T * sd) {
    ++effectTick;

    if (accTone == 0 || abs(accTone) < abs(step)) {
    	effectFunct = nullptr;
		accTone = 0;
		return;
    }

    if (effectTick >= effectArg[0]) {
    	accTone += step;
        effectTick = 0;
    }
    sd->toneShift += accTone;
}

void Channel::savePortParams() {
    tempAccTone = accTone;
    tempToneIdx = baseToneIdx;
}

#define YEStime effectArg[0]
#define NOtime effectArg[1]

void Channel::processVibrato(SAMPLE_DATA_T * sd) {

    if (effectTick < NOtime || (NOtime == 0 && effectTick >= YEStime)) {
        sd->volume = 0;
    }
    ++effectTick;
    if (effectTick > YEStime + NOtime && NOtime != 0) {
    	effectTick = 0;
    }
}

void Channel::processChanTick(uint8_t * freqH, uint8_t * freqL, uint8_t * mixer,
		uint8_t * vol, uint8_t * noise, uint16_t * env, uint8_t chipId) {
    SAMPLE_DATA_T sampData;
    sample->processSample(&sampData);
    if (effectFunct != nullptr) {
        (this->*effectFunct)(&sampData);
    }

    // This is wrong. The behavior on ornament > than table needs more investigation
    int16_t toneIdx = baseToneIdx + (int8_t)ornament->getSemitoneOffset();
    if (toneIdx < 0) {
        toneIdx = 0;
    } else if (toneIdx > 95) {
        toneIdx = 95;
    }
    uint16_t tone = freqTable[toneIdx] + sampData.toneShift;

    // The mystery of the tone/noiseless envelope? -> Yes. Kind of. ONLY on AY8930 chip!!!!
    // Will break AY-3-8913, since the tone period of 1 is way too short to sync
    // OK WITH YM2149F!!! KEEP THIS ONE IN

    if (chipId == 0x4) {
		if (sampData.mask == 0x09) {
			tone = 1;
			sampData.mask = 0x08;
		}
    }

    *freqH = (tone >> 8) & 0x0F;
    *freqL = tone & 0xFF;
    *mixer |= (sampData.mask << channel);

    if ((sampData.volume & 0x10) && masterEnvelope == 0) {
        *vol = 0x10;
    } else {
        *vol = volTable[volume][sampData.volume & 0x0F];
    }

    if (sampData.noiseOffset != 0)
        *noise = sampData.noiseOffset;
    if (sampData.envelopeOffset != 0 && masterEnvelope == 0)
        *env = sampData.envelopeOffset;
}

bool Channel::isBlank() {
    if (++inactCounter < inactiveLines) {
        return true;
    }
    inactCounter = 0;
    return false;
}

bool Channel::isPlaying() {
    return playing;
}

void Channel::mute() {
    playing = false;
}

void Channel::unmute() {
    playing = true;
}
