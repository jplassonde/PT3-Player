#pragma once

#include <cstdint>

class FrameCounter {
public:
	FrameCounter();
	virtual ~FrameCounter();
	void incCounter();
	uint8_t getCount();
	void latchCounter();
private:
	volatile uint8_t counter;
	volatile uint8_t count;
};
