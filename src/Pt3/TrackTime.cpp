#include "TrackTime.h"

TrackTime::TrackTime() {
    _totalTime = 0;
    _currentTime = 0;
}

TrackTime::~TrackTime() {}

void TrackTime::setTotal(uint32_t totalTime) {
    _totalTime = totalTime;
}

void TrackTime::setCurrent(uint32_t currentTime) {
    _currentTime = currentTime;
}
void TrackTime::incCurrent() {
    _currentTime += 20;
}

uint32_t TrackTime::getTotal() {
    return _totalTime;
}
uint32_t TrackTime::getCurrent() {
    return _currentTime;
}
