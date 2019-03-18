#pragma once

#include <cstdint>

constexpr uint8_t NO_GESTURE 	= 0;
constexpr uint8_t SWIPE_UP 		= 1;
constexpr uint8_t SWIPE_LEFT 	= 2;
constexpr uint8_t SWIPE_DOWN 	= 3;
constexpr uint8_t SWIPE_RIGHT	= 4;

constexpr uint8_t EVENT_DOWN 	= 0;
constexpr uint8_t EVENT_UP 		= 1;
constexpr uint8_t EVENT_CONTACT = 2;
constexpr uint8_t NO_EVENT 		= 3;

typedef struct TOUCH_EVENT_T {
	uint8_t gesture;
	uint16_t gestureMagnitude;
	uint8_t touchEvent;
	uint16_t xPosition;
	uint16_t yPosition;
} TOUCH_EVENT_T;
