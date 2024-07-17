#pragma once

#include <cstdint>

//
#define BUFFER1 0xC0000000
#define BUFFER2 0xC0000000 + 4*480*800

class Screen {
public:
	static void refresh();
	static void swapBuffers();
	static uint32_t getBackBufferAddr();
private:
	static uint32_t backBuffer;
	static uint32_t frontBuffer;
	Screen() {}
};
