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

Pt3Parser::Pt3Parser(SOUNDCHIP_T * const chip, const uint8_t * module, bool loop) :
														  soundchip(chip),
														  module(module), loop(loop),
													   	  pTable((uint16_t *)&module[*(uint16_t *)&module[PTABLE_LOC]]),
														  ornTable((uint16_t *)&module[ORNTABLE_IDX]),
														  sampleTable((uint16_t *)&module[SAMPTABLE_IDX]),
														  patternOrder(&module[PATTERNORDER_IDX]) {
	speed = module[TEMPO_IDX];
	tick = speed;
	currentPattern = 0;
	baseNoise = 0;
	baseEnvelope = 0;
	envAcc = 0;
	envSlideEnabled = false;

	const uint16_t * ft = freqTables[module[99]];

	// Init all three channels with 1st pattern and 1st sample (0th never used) and 0th ornament
	channelA = new Channel(Channel::chanA, pTable[patternOrder[currentPattern]], &module[sampleTable[1]], &module[ornTable[0]], ft);
	channelB = new Channel(Channel::chanB, pTable[patternOrder[currentPattern]+1], &module[sampleTable[1]], &module[ornTable[0]], ft);
	channelC = new Channel(Channel::chanC, pTable[patternOrder[currentPattern]+2], &module[sampleTable[1]], &module[ornTable[0]], ft);
}

Pt3Parser::~Pt3Parser() {
	delete channelA;
	delete channelB;
	delete channelC;
}

bool Pt3Parser::processTick() {
	uint16_t envelopeFreq = 0;
	uint8_t noiseFreq = 0;

	if ( tick >= speed ) { // Signal a new line has to be parsed

		if (!channelA->isBlank()) {
			if (module[channelA->pos] == 0) { // 0 at the start of Channel line indicates end of pattern.
				if (!advancePattern()) { // If the end is reached and loop is off
					return false;		 // return false, end track.
				}
			}
			while(parseLine(channelA));

		}
		if (!channelB->isBlank())
			while(parseLine(channelB));
		if (!channelC->isBlank())
			while(parseLine(channelC));
		tick = 0;
	}

	soundchip->mixer = 0;

	if (channelA->isPlaying())
		channelA->processChanTick(&soundchip->chanFreqAH, &soundchip->chanFreqAL,
								  &soundchip->mixer, &soundchip->volA,
								  &noiseFreq, &envelopeFreq);
	else
		soundchip->volA = 0;

	if (channelB->isPlaying())
		channelB->processChanTick(&soundchip->chanFreqBH, &soundchip->chanFreqBL,
								  &soundchip->mixer, &soundchip->volB,
								  &noiseFreq, &envelopeFreq);
	else
		soundchip->volB = 0;

	if (channelC->isPlaying())
		channelC->processChanTick(&soundchip->chanFreqCH, &soundchip->chanFreqCL,
								  &soundchip->mixer, &soundchip->volC,
								  &noiseFreq, &envelopeFreq);
	else
		soundchip->volC = 0;

	//TEST - REMOVE
	//soundchip->mixer = 0xFD;
	//soundchip->mixer = 0x3F;
	//soundchip->volA &= 0x10;

	if (envSlideEnabled) {
		processEnvSlide();
	}
	if (baseNoise == 0) {
		baseNoise =1;
	}

	envelopeFreq += baseEnvelope;
	noiseFreq += baseNoise;
	if (envelopeFreq == 0) {
		envelopeFreq = 1;
	}
	soundchip->envH = (envelopeFreq & 0xFF00) >> 8;
	soundchip->envL = envelopeFreq & 0xFF;
	soundchip->noiseFreq = noiseFreq & 0x1F;
	++tick;
	return true;
}

bool Pt3Parser::advancePattern() {
	uint8_t patternNumber = patternOrder[++currentPattern];

	if (currentPattern >= module[PATTERNCNT_IDX]) { // When it reached the end
		if (loop == false) {
			return false;
		}
		currentPattern = module[LOOPPOS_IDX];
		patternNumber = patternOrder[currentPattern];
	}

	channelA->pos = pTable[patternNumber];
	channelB->pos = pTable[patternNumber+1];
	channelC->pos = pTable[patternNumber+2];

	return true;
}


uint8_t Pt3Parser::parseLine(Channel * channel) {
	uint8_t data = module[channel->pos++];

	switch(data) {

	case 0x00: // Means end of track. should never get here.
		while(1);

	case 0x01: // Glissando
		while(parseLine(channel)); // Process other bytes of data until we get to args
		channel->setGliss(&module[channel->pos]);
		channel->pos+=3;
		return 0;

	case 0x02: // Portamento
		channel->savePortParams();
		while(parseLine(channel));
		channel->setPortamento(&module[channel->pos]);
		channel->pos+=5;
		return 0;

	case 0x03: // Sample Offset
		while(parseLine(channel));
		channel->setSampleOffset(module[channel->pos]);
		++channel->pos;
		return 0;

	case 0x04: // Ornament offset
		while(parseLine(channel));
		channel->setOrnamentOffset(module[channel->pos++]);
		return 0;

	case 0x05: // Vibrato
		while(parseLine(channel));
		channel->setVibrato(&module[channel->pos]);
		channel->pos+=2;
		return 0;

	case 0x08: // Slide Envelope
		while(parseLine(channel));
		setEnvelopeSlide(&module[channel->pos]);
		channel->pos+=3;
		return 0;

	case 0x09: // Set Speed
		while(parseLine(channel));
		speed = module[channel->pos++];
		return 0;


	case 0x10: // Undocumented. pt 3.6 feature?
		channel->disableEnvelope();
		channel->setSample(&module[sampleTable[module[channel->pos++]/2]]);
		return 1;

	case 0x11 ... 0x1E: // Init envelope and sample
		initEnvelope((data & 0x0F)/*-1*/, module[channel->pos], module[channel->pos+1], channel);
		channel->setSample(&module[sampleTable[module[channel->pos+2]/2]]);
		channel->pos+=3;
		return 1;
	case 0x1F:
		while(1); // Find out if that case is ever used (and where/how/why)

	case 0x20 ... 0x3F: // Base noise
		baseNoise = data - 20;
		return 1;

	case 0x40 ... 0x4F: // Select ornament
		channel->setOrnament(&module[ornTable[data & 0x0F]]);
		return 1;

	case 0x50 ... 0xAF: // Set pitch index (freq table) and end line parsing
		channel->setTone(data-0x50);
		channel->unmute();

		return 0;

	case 0xB0: // Turn off envelope
		channel->disableEnvelope();
		return 1;

	case 0xB1: // Set number of blank lines
		channel->setBlanks(module[channel->pos++]);
		return 1;

	case 0xB2 ... 0xBF: // Envelope Selection
		initEnvelope((data & 0x0F) - 1, module[channel->pos], module[channel->pos+1], channel);
		channel->pos+=2;
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
		channel->setSample(&module[sampleTable[data-0xD0]]);
		return 1;

	case 0xF0 ... 0xFF: // Set ornament & sample
		channel->setOrnament(&module[ornTable[data & 0x0F]]);
		channel->setSample(&module[sampleTable[module[channel->pos++]/2]]);
		channel->disableEnvelope();
		return 1;
	default:
		while(1); // Should never get here -> debug
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
		envTick=0;
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
