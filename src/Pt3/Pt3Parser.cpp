#include "Pt3Parser.h"
#include "Channel.h"

#include "freqTables.h"

constexpr uint8_t TEMPO_IDX = 0x64;
constexpr uint8_t PATTERNCNT_IDX = 0x65;
constexpr uint8_t LOOPPOS_IDX = 0x66;
constexpr uint8_t PTABLE_LOC = 0x67;
constexpr uint8_t SAMPTABLE_IDX = 0x69;
constexpr uint8_t ORNTABLE_IDX = 0xA9;
constexpr uint8_t PATTERNORDER_IDX = 0xC9;

Pt3Parser::Pt3Parser(SOUNDCHIP_T * const chip, const uint8_t * module)
        : soundchip(chip),
            module(module),
            pTable((uint16_t *)&module[*(uint16_t *)&module[PTABLE_LOC]]),
            ornTable((uint16_t *)&module[ORNTABLE_IDX]),
            sampleTable((uint16_t *)&module[SAMPTABLE_IDX]),
            patternOrder(&module[PATTERNORDER_IDX]),
            channelA(nullptr),
            channelB(nullptr),
            channelC(nullptr) {

    reset();
}

Pt3Parser::~Pt3Parser() {
    delete channelA;
    delete channelB;
    delete channelC;
}

uint8_t Pt3Parser::processTick() {
    uint16_t envelopeFreq = 0;
    uint8_t noiseFreq = 0;

    if (tick >= speed) { // Signal a new line has to be parsed

        if (!channelA->isBlank()) {
            if (module[channelA->pos] == 0) { // 0 at the start of Channel line indicates end of pattern.
                if (advancePattern() == false) { // If the end is reached
                    channelA->setBlanks(0);
                    return false;		 // return false, end track.
                }
            }
            while (parseLine(channelA));
        }

        if (!channelB->isBlank())
            while (parseLine(channelB));
        if (!channelC->isBlank())
            while (parseLine(channelC));
        tick = 0;
    }

    soundchip->mixer = 0;

    if (channelA->isPlaying())
        channelA->processChanTick(&soundchip->chanFreqAH,
                &soundchip->chanFreqAL, &soundchip->mixer, &soundchip->volA,
                &noiseFreq, &envelopeFreq, soundchip->mask);
    else
        soundchip->volA = 0;

    if (channelB->isPlaying())
        channelB->processChanTick(&soundchip->chanFreqBH,
                &soundchip->chanFreqBL, &soundchip->mixer, &soundchip->volB,
                &noiseFreq, &envelopeFreq, soundchip->mask);
    else
        soundchip->volB = 0;

    if (channelC->isPlaying())
        channelC->processChanTick(&soundchip->chanFreqCH,
                &soundchip->chanFreqCL, &soundchip->mixer, &soundchip->volC,
                &noiseFreq, &envelopeFreq, soundchip->mask);
    else
        soundchip->volC = 0;

    if (envSlideEnabled) {
        processEnvSlide();
    }

    if (envelopeFreq != 0) {
    	envelopeFreq++;
    }

    envelopeFreq += baseEnvelope;
    noiseFreq += baseNoise;

    if (noiseFreq == 0) // Is it AY8930 specific? Test with YM2149/AY-3-8910 to see if it glitches too at 0 freq <- It doesnt. Lets keep it there though
        noiseFreq = 1;

    if (envelopeFreq == 0)
        envelopeFreq = 1;

    soundchip->envH = (envelopeFreq & 0xFF00) >> 8;
    soundchip->envL = envelopeFreq & 0xFF;
    soundchip->noiseFreq = noiseFreq & 0x1F;
    ++tick;
    return true;
}

bool Pt3Parser::advancePattern() {
    bool retval = true;
    uint8_t patternNumber = patternOrder[++currentPattern];

    if (currentPattern >= module[PATTERNCNT_IDX]) { // When it reached the end
        retval = false;
        currentPattern = module[LOOPPOS_IDX];
        patternNumber = patternOrder[currentPattern];
    }

    channelA->pos = pTable[patternNumber];
    channelB->pos = pTable[patternNumber + 1];
    channelC->pos = pTable[patternNumber + 2];

    return retval;
}

uint8_t Pt3Parser::parseLine(Channel * channel) {
    uint8_t data = module[channel->pos++];

    switch (data) {

        case 0x00: // Means end of track. should never get here.
            while (1); // Hang->Debug, find out why.
            break;
        case 0x01: // Glissando
            while (parseLine(channel)); // Process other bytes of data until we get to args
            channel->setGliss(&module[channel->pos]);
            channel->pos += 3;
            return 0;

        case 0x02: // Portamento
            channel->savePortParams();
            while (parseLine(channel));
            channel->setPortamento(&module[channel->pos]);
            channel->pos += 5;
            return 0;

        case 0x03: // Sample Offset
            while (parseLine(channel));
            channel->setSampleOffset(module[channel->pos]);
            ++channel->pos;
            return 0;

        case 0x04: // Ornament offset
            while (parseLine(channel));
            channel->setOrnamentOffset(module[channel->pos++]);
            return 0;

        case 0x05: // Vibrato
            while (parseLine(channel));
            channel->setVibrato(&module[channel->pos]);
            channel->pos += 2;
            return 0;

        case 0x08: // Slide Envelope
            while (parseLine(channel));
            setEnvelopeSlide(&module[channel->pos]);
            channel->pos += 3;
            return 0;

        case 0x09: // Set Speed
            while (parseLine(channel));
            speed = module[channel->pos++];
            return 0;

        case 0x10: // Undocumented. pt 3.6 feature?
            channel->disableEnvelope();
            channel->setSample(&module[sampleTable[module[channel->pos++] / 2]]);
            return 1;

        case 0x11 ... 0x1E: // Init envelope and sample
            initEnvelope((data & 0x0F)/*-1*/, module[channel->pos],
                    module[channel->pos + 1], channel);
            channel->setSample(&module[sampleTable[module[channel->pos + 2] / 2]]);
            channel->pos += 3;
            return 1;

        case 0x1F:
            while (1) ; // Find out if that case is ever used (and where/how/why)
            break;
        case 0x20 ... 0x3F: // Base noise
            baseNoise = data - 0x20;
            return 1;

        case 0x40 ... 0x4F: // Select ornament
            channel->setOrnament(&module[ornTable[data & 0x0F]]);
            return 1;

        case 0x50 ... 0xAF: // Set pitch index (freq table) and end line parsing
            channel->setTone(data - 0x50);
            channel->unmute();
            return 0;

        case 0xB0: // Turn off envelope
            channel->disableEnvelope();
            return 1;

        case 0xB1: // Set number of blank lines
            channel->setBlanks(module[channel->pos++]);
            return 1;

        case 0xB2 ... 0xBF: // Envelope Selection
            initEnvelope((data & 0x0F) - 1, module[channel->pos], module[channel->pos + 1], channel);
            channel->pos += 2;
            return 1;

        case 0xC0:  // Turn off channel and end line parsing
            channel->mute();
            return 0;

        case 0xC1 ... 0xCF: // Volume Selection
            channel->setBaseVol(data & 0x0F);
            return 1;

        case 0xD0:  // End line parsing
            return 0;

        case 0xD1 ... 0xEF: // Set sample #
            channel->setSample(&module[sampleTable[data - 0xD0]]);
            return 1;

        case 0xF0 ... 0xFF: // Set ornament & sample
            channel->setOrnament(&module[ornTable[data & 0x0F]]);
            channel->setSample(&module[sampleTable[module[channel->pos++] / 2]]);
            channel->disableEnvelope();
            return 1;
        default:
            while (1); // Should never get here -> debug
    }
    return 0;
}

void Pt3Parser::setEnvelopeSlide(const uint8_t * const data) {
    envDelay = data[0];
    envSlide = data[1] | (data[2] << 8);
    envSlideEnabled = true;
    envTick = 0;
}

void Pt3Parser::processEnvSlide() {
    if (envTick >= envDelay) {
        baseEnvelope += envSlide;
        envTick = 0;
    }
    ++envTick;
}

void Pt3Parser::initEnvelope(uint8_t shape, uint8_t envH, uint8_t envL, Channel * channel) {
    soundchip->envShape = shape;
    baseEnvelope = (envH << 8) | envL;
    channel->enableEnvelope();
    envSlideEnabled = false;
    soundchip->envReset = 1;
}

uint32_t Pt3Parser::getTotalTime() {
    uint32_t totalTime = 0;

    while (processTick()) {
        totalTime += 20;
    }

    reset();
    return totalTime;
}

uint32_t Pt3Parser::getLoopTime() {
    uint32_t loopTime = 0;
    while (currentPattern != module[LOOPPOS_IDX]) {
        processTick();
        loopTime += 20;
    }
    reset();
    return loopTime;
}

void Pt3Parser::reset() {
    delete channelA;
    delete channelB;
    delete channelC;

    speed = module[TEMPO_IDX];
    tick = speed;
    currentPattern = 0;
    baseNoise = 0;
    baseEnvelope = 0;
    envAcc = 0;
    envSlideEnabled = false;
    speed = module[TEMPO_IDX];

    const uint16_t * ft;
    // For table 2, use 2.5  if the its a Vortex II file or pro tracker 3.5+
    if (module[99] == 2 && (module[13] >= 35 || module[0] == 'V')) {
    	ft = freqTable2_5;
    } else {
    	ft = freqTables[module[99]];
    }

    channelA = new Channel(Channel::chanA, pTable[patternOrder[currentPattern]],
            &module[sampleTable[1]], &module[ornTable[0]], ft);

    channelB = new Channel(Channel::chanB,
            pTable[patternOrder[currentPattern] + 1], &module[sampleTable[1]],
            &module[ornTable[0]], ft);

    channelC = new Channel(Channel::chanC,
            pTable[patternOrder[currentPattern] + 2], &module[sampleTable[1]],
            &module[ornTable[0]], ft);
}
