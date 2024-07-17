
#define PLAYING 0
class TrackInfos;

typedef struct IEVENT_t {
	uint8_t type;
	union {
		TrackInfos * trackInfos;
	};
} IEVENT_t;
