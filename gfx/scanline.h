#include <cstdint>

uint8_t scanline[] = {0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,0xAF,
		0xAF,0xAF,0xAF};
