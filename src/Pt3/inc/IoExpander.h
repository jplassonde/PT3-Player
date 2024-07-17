#pragma once

constexpr uint8_t IOA = 0x12;
constexpr uint8_t IOB = 0x13;
constexpr uint8_t BDIR = 0x01;
constexpr uint8_t BC1 =  0x02;
constexpr uint8_t AY13 = 0x08;
constexpr uint8_t AY30 = 0x04;
class IoExpander {
public:
	IoExpander();
	virtual ~IoExpander();
	void reset();
	void sendData(uint8_t * data, uint16_t size);
	static void sendDataPolling(uint8_t * data, uint16_t size);
};
