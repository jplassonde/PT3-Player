#include <TrackInfos.h>
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"

TrackInfos::TrackInfos(const char * filename, const char * trackName, const char * author) {
    _filename = (char *)pvPortMalloc((strlen(filename) + 1) * sizeof(char));
    strcpy(_filename, filename);

    setName(&_trackName, trackName);
    setName(&_author, author);
}

TrackInfos::~TrackInfos() {
    vPortFree(_filename);
    vPortFree(_trackName);
    vPortFree(_author);
}

// Copy the author/module name up to the last space present
void TrackInfos::setName(char ** to, const char * from) {
    for (uint8_t i = 31; i >= 0; i--) {
        if (from[i] != ' ' || i == 0) {
            ++i;
            *to = (char *)pvPortMalloc((i + 1) * sizeof(char));
            strncpy(*to, from, i);
            (*to)[i] = 0;
            return;
        }
    }
}

const char * TrackInfos::getFilename() {
    return _filename;
}
const char * TrackInfos::getTrackName() {
    return _trackName;
}

const char * TrackInfos::getAuthor() {
    return _author;
}
