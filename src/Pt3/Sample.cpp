#include <Sample.h>

constexpr uint8_t LOOP_IDX = 0;
constexpr uint8_t END_IDX = 1;

#define NOISE_MASK 	(sampleBase[pos+1] & 0x80)
#define NEACC_ON 	(sampleBase[pos+1] & 0x20)
#define NE_OFFSET 	((sampleBase[pos] >> 1) & 0x0F)
#define ENV_MASK	((sampleBase[pos]) & 0x01)
#define TONE_ACC 	(sampleBase[pos+1] & 0x40)
#define VOLBITS 	(sampleBase[pos+1] & 0x0F)
#define VOL_SIGN	(sampleBase[pos] & 0xC0)
#define VOL_INCREASE 0xC0
#define VOL_DECREASE 0x80

Sample::Sample(const uint8_t * base) {
    initSample(base);
}

Sample::~Sample() {
}

void Sample::processSample(SAMPLE_DATA_T * sampleData) {
    if (idx >= end)
        idx = loop;
    if (sampleBase == nullptr)
        while (1); // should not happen -> hang and debug

    const uint8_t pos = idx * 4;

// Set Noise or Envelope offset (depend on noise mask)
    sampleData->noiseOffset = 0;
    sampleData->envelopeOffset = 0;

    if (NOISE_MASK == 0) {
        sampleData->noiseOffset = accNoise + NE_OFFSET;
        if (NEACC_ON) {
            accNoise += NE_OFFSET;
        }
    } else if (ENV_MASK == 0) {
        sampleData->envelopeOffset = accEnv + NE_OFFSET;
        if (NEACC_ON) {
            accEnv += NE_OFFSET;
        }
    } else if (NEACC_ON) { // No ~mask on, but accumulation-> increase env offset
        accEnv += NE_OFFSET;
    }

// Set Tone Shift
    uint16_t tone = *(uint16_t *)&sampleBase[pos + 2];
    sampleData->toneShift = accTone + tone;
    if (TONE_ACC) {
        accTone += tone;
    }

// Calculate Volume
    if ((VOL_SIGN == VOL_INCREASE) && volModif < 15) {
        ++volModif;
    } else if ((VOL_SIGN == VOL_DECREASE) && volModif > -15) {
        --volModif;
    }
    int8_t vol = VOLBITS + volModif;
    if (vol > 0x0F)
        vol = 0x0F;
    else if (vol < 0)
        vol = 0;

    // Add volume mode bit
    vol = vol | ((~sampleBase[pos] & 0x01) << 4);
    sampleData->volume = (uint8_t)vol;

// Set Channel Mask
    sampleData->mask = (sampleBase[pos + 1] & 0x90) >> 4;

    ++idx; // increment pos index and done
}

void Sample::initSample(const uint8_t * base) {
    loop = base[LOOP_IDX];
    end = base[END_IDX];
    sampleBase = base + 2;
    reset();
}

void Sample::setOffset(uint8_t offset) {
    idx = offset;
}

void Sample::reset() {
    accEnv = 0;
    accNoise = 0;
    accTone = 0;
    volModif = 0;
    idx = 0;
}
