#pragma once

#include <cstdint>

class TrackTime {
public:
	TrackTime();
	virtual ~TrackTime();
	void setTotal(uint32_t totalTime);
	void setCurrent(uint32_t currentTime);
	void incCurrent();
	uint32_t getTotal();
	uint32_t getCurrent();
private:
	uint32_t _totalTime __attribute__ ((aligned (32)));
	uint32_t _currentTime __attribute__ ((aligned (32)));
};
