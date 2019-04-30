#pragma once

class TrackInfos {
public:
	TrackInfos(const char * filename, const char * trackName, const char * author);
	virtual ~TrackInfos();
	const char * getFilename();
	const char * getTrackName();
	const char * getAuthor();
private:
	void setName(char ** to, const char * from);
	char * _filename;
	char * _trackName;
	char * _author;
};
