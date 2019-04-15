#pragma once

#include <map>
#include <cstdint>


void fontInit();

class Font {

public:
	static void init();
	enum fontSelect {rez18px, rez27px};
	Font(fontSelect fontSel);
	virtual ~Font();
	const uint8_t * getChar(uint8_t c);
	uint8_t getWidth() const;
	uint8_t getHeight() const;
private:
	std::map<char, const uint8_t *> * fontMap;
	uint8_t height;
	uint8_t width;
	const uint8_t * defaultChar;
};

