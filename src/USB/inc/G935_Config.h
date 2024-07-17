#pragma once

#include <cstdint>

/*
 * Commands to enter / set configuration mode in the G935 headset.
 * All the commands are 20 bytes long, but to save space, the arrays
 * begin with the number of non-zero bytes, the rest of the padding is
 * to be taken care by the application.
 *
 * These configs are necessary to fake Logitech G HUB, otherwise the headset
 * buttons, EQ and leds are set to an internal default behavior.
 *
 * A good amount of these commands seems to only be used to retrieve current
 * settings from the headset, trial and error could be used to reduce the list length.
 *
 */

const uint8_t cmd1[] = {4, 0x11, 0xff, 0x00, 0x1c};   					// 11 FF 00 1D 04 02
const uint8_t cmd2[] = {7, 0x11, 0xff, 0x00, 0x1c, 0x00, 0x00, 0x39}; 	// 11 FF 00 1D 04 02 39
const uint8_t cmd3[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x00, 0x03}; 		// 11 FF 00 0D 02 00 02
const uint8_t cmd4[] = {4, 0x11, 0xff, 0x02, 0x0c};						// 11 FF 02 0D 01 FF FF FF FF 00 03 00 00 00 00 0A 87
const uint8_t cmd5[] = {4, 0x11, 0xff, 0x02, 0x1c};						// 11 FF 02 1D 00 55 31 20 29 00 00 12 01 0A 87
const uint8_t cmd6[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x1F, 0x20};			// 11 FF 00 0D 08 00 03
const uint8_t cmd7[] = {4, 0x11, 0xff, 0x08, 0x0c};						// 11 FF FF 08 0D 05
const uint8_t cmd8[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x00, 0x20};    		// 11 FF 00 0D
const uint8_t cmd9[] = {5, 0x11, 0xff, 0x00, 0x0c, 0x10};           	// 11 FF 00 0D
const uint8_t cmd10[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x10, 0x01};		// 11 FF 00 0D
const uint8_t cmd11[] = {4, 0x11, 0xff, 0x08, 0x0c};					// 11 FF FF 08 0D 05
// Poll battery val here? There are 3 interrupt IN going in here usually
// This is also where interface 2, alt setting 2 is set, along with the 44100 freq.

const uint8_t cmd11_1[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x1f, 0x20}; 		// 11 FF 00 0C 08 00 03
const uint8_t cmd11_2[] = {4, 0x11, 0xff, 0x08, 0x0c};					// 11 FF 08 0D XX XX 01 - Battery lvl

const uint8_t cmd12[] = {5, 0x11, 0xff, 0x00, 0x0c, 0x81};				// 11 FF 00 0D
const uint8_t cmd13[] = {5, 0x11, 0xff, 0x00, 0x0c, 0x13};				// 11 FF 00 0D
const uint8_t cmd14[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x40};		// 11 FF 00 0D
const uint8_t cmd15[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x71};		// 11 FF 00 0D
const uint8_t cmd16[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x70};		// 11 FF 00 0D 04 00 03

																		// These values appears to be current config. Are bound to change
const uint8_t cmd17[] = {4, 0x11, 0xff, 0x04, 0x0c};					// 11 FF 04 0D 02 00 01 00 03
const uint8_t cmd18[] = {4, 0x11, 0xff, 0x04, 0x1c};					// 11 FF 04 1D 00 00 02 04
const uint8_t cmd19[] = {4, 0x11, 0xff, 0x04, 0x2c};					// 11 FF 04 2D
const uint8_t cmd20[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x01};		// 11 FF 2D 00 01 00 01
const uint8_t cmd21[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x02};		// 11 FF 04 2D 00 02 00 0A c1 15 00 04
const uint8_t cmd22[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x03};		// 11 FF 04 2D 00 03 00 03 c0 05 00 06
const uint8_t cmd23[] = {5, 0x11, 0xff, 0x04, 0x1c, 0x01};				// 11 FF 04 1D 01 00 01 04
const uint8_t cmd24[] = {5, 0x11, 0xff, 0x04, 0x2c, 0x01};				// 11 FF 04 2D 01
const uint8_t cmd25[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x01};		// 11 FF 04 2D 01 01 00 01
const uint8_t cmd26[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x02};		// 11 FF 04 2D 01 02 00 0A c1 15 00 04
const uint8_t cmd27[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x03};		// 11 FF 04 2D 01 03 00 03 c0 05 00 06
const uint8_t cmd28[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x70};		// 11 FF 00 0D 04 00 03

// Same sequence a 2nd time, with a different ending					-  Expect same replies as above
const uint8_t cmd29[] = {4, 0x11, 0xff, 0x04, 0x0c};					// 11 FF 04 0D 02 00 01 00 03
const uint8_t cmd30[] = {4, 0x11, 0xff, 0x04, 0x1c};					// 11 FF 04 1D 00 00 02 04
const uint8_t cmd31[] = {4, 0x11, 0xff, 0x04, 0x2c};					// 11 FF 04 2D
const uint8_t cmd32[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x01};		// 11 FF 2D 00 01 00 01
const uint8_t cmd33[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x02};		// 11 FF 04 2D 00 02 00 0A c1 15 00 04
const uint8_t cmd34[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x00, 0x03};		// 11 FF 04 2D 00 03 00 03 c0 05 00 06
const uint8_t cmd35[] = {5, 0x11, 0xff, 0x04, 0x1c, 0x01};				// 11 FF 04 1D 01 00 01 04
const uint8_t cmd36[] = {5, 0x11, 0xff, 0x04, 0x2c, 0x01};				// 11 FF 04 2D 01
const uint8_t cmd37[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x01};		// 11 FF 04 2D 01 01 00 01
const uint8_t cmd38[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x02};		// 11 FF 04 2D 01 02 00 0A c1 15 00 04
const uint8_t cmd39[] = {6, 0x11, 0xff, 0x04, 0x2c, 0x01, 0x03};		// 11 FF 04 2D 01 03 00 03 c0 05 00 06
const uint8_t cmd40[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x81};		// 11 ff 00 0D
const uint8_t cmd41[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x80};		// 11 ff 00 0D
const uint8_t cmd42[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x18, 0x15};		// 11 ff 00 0D
const uint8_t cmd43[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x10};		// 11 ff 00 0D 05

const uint8_t cmd44[] = {4, 0x11, 0xff, 0x05, 0x0c};					// 11 FF 05 0D 03
const uint8_t cmd45[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x81, 0x10};		// 11 FF 00 0D
const uint8_t cmd46[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x20};		// 11 ff 00 0D
const uint8_t cmd47[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x30};		// 11 FF 00 0D
const uint8_t cmd48[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x60};		// 11 ff 00 0D
const uint8_t cmd49[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x40, 0xA2};		// 11 ff 00 0D
const uint8_t cmd50[] = {5, 0x11, 0xff, 0x00, 0x0c, 0x24};				// 11 ff 00 0D
const uint8_t cmd51[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x83, 0x10};		// 11 ff 00 0D 06 00 01
const uint8_t cmd52[] = {5, 0x11, 0xff, 0x00, 0x0c, 0x83};				// 11 FF 00 0D 07

const uint8_t cmd53[] = {4, 0x11, 0xff, 0x06, 0x0c};					// 11 FF 06 0D 0A 0C 01
const uint8_t cmd54[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x22, 0x40};		// 11 FF 00 0D
const uint8_t cmd55[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x00, 0xc1};		// 11 FF 00 0D
const uint8_t cmd56[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x00, 0xc2};		// 11 FF 00 0D
const uint8_t cmd57[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x81, 0x20};		// 11 FF 00 0D
const uint8_t cmd58[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x81, 0x23};		// 11 FF 00 0D
const uint8_t cmd59[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0xa3};		// 11 FF 00 0D
const uint8_t cmd60[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0xd0};		// 11 FF 00 0D
const uint8_t cmd61[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x80, 0x7a};		// 11 FF 00 0D

const uint8_t cmd62[] = {4, 0x11, 0xff, 0x08, 0x0c}; // Checking the batterie twice here. Every time.
const uint8_t cmd63[] = {4, 0x11, 0xff, 0x08, 0x0c}; // Could probably be cut

//const uint8_t cmd64[] = {5, 0x11, 0xff, 0x04, 0xcd, 0x01}; // This is for adaptive light? We roll our own here...
const uint8_t cmd64[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x45, 0x22}; 		// 11 FF 00 0D
const uint8_t cmd65[] = {5, 0x11, 0xff, 0x04, 0xcc, 0x01};				// 11 FF FF 04 CD 07

const uint8_t cmd66[] = {5, 0x11, 0xff, 0x05, 0x2c, 0x01};				// 11 FF 05 2D 01
const uint8_t cmd67[] = {4, 0x11, 0xff, 0x04, 0xcc};					// 11 FF FF 04 CD 07
const uint8_t cmd68[] = {5, 0x11, 0xff, 0x04, 0xcc, 01};				// 11 FF FF 04 CD 07
const uint8_t cmd69[] = {4, 0x11, 0xff, 0x04, 0xcc};					// 11 FF FF 04 CD 07

const uint8_t cmd70[] = {4, 0x11, 0xff, 0x04, 0x7c};					// 11 FF FF 04 7D 07
const uint8_t cmd71[] = {6, 0x11, 0xff, 0x04, 0x8c, 0x01, 0x01};		// 11 FF 04 8D 01 01
const uint8_t cmd72[] = {5, 0x11, 0xff, 0x04, 0xec, 0x01}; 				// 11 FF 04 ED 01 02 FF 00 00 1F 40 06 64
const uint8_t cmd73[] = {5, 0x11, 0xff, 0x04, 0xec, 0x01};				// 11 FF 04 ED 01 02 FF 00 00 1F 40 06 64

const uint8_t cmd74[] = {6, 0x11, 0xff, 0x00, 0x0c, 0x00, 0x05};		// 11 FF 00 0D 03
const uint8_t cmd75[] = {4, 0x11, 0xff, 0x03, 0x2c};					// 11 FF 03 2D 08

const uint8_t cmd76[] = {6, 0x11, 0xFF, 0x04, 0x4c, 0x00, 0x01};		// 11 FF 04 4C 02
const uint8_t cmd77[] = {4, 0x11, 0xff, 0x08, 0x1c};					// 11 FF 08 1D

// interface set here - bReq 1, wVal 0x100, wIdx 0x300, wlen 1, data 0

const uint8_t cmd78[] = {5, 0x11, 0xff, 0x04, 0xec, 0x01};			// 11 FF 04 ed 01 02 ff 00 00 1f 40 06 64

// Set ledstrips to white, logo off
const uint8_t cmd79[] = {10, 0x11, 0xFF, 0x04, 0x3c, 0x01, 0x01, 0x3f, 0x3f, 0x3f, 0x02};
const uint8_t cmd80[] = {6, 0x11, 0xFF, 0x04, 0x3c, 0x00, 0x00};

const uint8_t cmd81[] = {4, 0x11, 0xFF, 0x07, 0x0c}; 				// 11 ff 07 0d
const uint8_t cmd82[] = {4, 0x11, 0xFF, 0x06, 0x2c}; 				// Current EQ value
const uint8_t cmd83[] = {4, 0x11, 0xFF, 0x07, 0x1c}; 				// 11 ff 07 1d




#define G935_SETUP_CMD_COUNT 86
const uint8_t * g935Commands[86] = {cmd1, cmd1, cmd2, cmd3, cmd4, cmd5, cmd6, cmd7, cmd8, cmd9, cmd10,
									cmd11, cmd11_1, cmd11_2, cmd12, cmd13, cmd14, cmd15, cmd16, cmd17, cmd18, cmd19, cmd20,
									cmd21, cmd22, cmd23, cmd24, cmd25, cmd26, cmd27, cmd28, cmd29, cmd30,
									cmd31, cmd32, cmd33, cmd34, cmd35, cmd36, cmd37, cmd38, cmd39, cmd40,
									cmd41, cmd42, cmd43, cmd44, cmd45, cmd46, cmd47, cmd48, cmd49, cmd50,
									cmd51, cmd52, cmd53, cmd54, cmd55, cmd56, cmd57, cmd58, cmd59, cmd60,
									cmd61, cmd62, cmd63, cmd64, cmd65, cmd66, cmd67, cmd68, cmd69, cmd70,
									cmd71, cmd72, cmd73, cmd74, cmd75, cmd76, cmd77, cmd78, cmd79, cmd80,
									cmd81, cmd82, cmd83};


