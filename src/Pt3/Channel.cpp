#include "Channel.h"
#include <cmath>
#include <cstdlib>

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
    uint16_t toneDiff = freqTable[tempToneIdx] - freqTable[baseToneIdx];
    accTone += toneDiff;
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

void Channel::processPortamento(SAMPLE_DATA_T * sd) {
    ++effectTick;
    if ((effectArg[4] & 0x80 && accTone + *(int16_t*)&effectArg[3] < 0)
            || ((!(effectArg[4] & 0x80)
                    && accTone - *(int16_t*)&effectArg[3] < 0))) {
        effectFunct = nullptr;
        accTone = 0;
        return;
    }
    if (effectTick >= effectArg[0]) {
        accTone += *(uint16_t*)&effectArg[3];
        effectTick = 0;
    }
    sd->toneShift += accTone;
}

void Channel::savePortParams() {
    tempAccTone = accTone;
    tempToneIdx = baseToneIdx;
}

void Channel::processVibrato(SAMPLE_DATA_T * sd) {
    if (effectTick >= effectArg[0]) {
        sd->volume = 0;
    }
    if (effectTick >= effectArg[0] + effectArg[1] && effectArg[1] != 0) {
        effectTick = 0;
    }
    ++effectTick;
}

void Channel::processChanTick(uint8_t * freqH, uint8_t * freqL, uint8_t * mixer, uint8_t * vol, uint8_t * noise, uint16_t * env) {
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

    // The mystery of the tone/noiseless envelope? -> Yes. Kind of
    if (sampData.mask == 0x09) {
        tone = 1;
        sampData.mask = 0x08;
    }

    *freqH = (tone >> 8) & 0x0F;
    *freqL = tone & 0xFF;
    *mixer |= (sampData.mask << channel);

    if (sampData.volume & 0x10 && masterEnvelope == 0) {
        *vol = 0x10;
    } else {
        *vol = (uint8_t)round(((sampData.volume & 0x0F) * volume) / 0x0F) & 0x0F;
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
