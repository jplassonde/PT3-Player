#ifndef HW_CONF_GUARD
#define HW_CONF_GUARD
#include "otm8009a.h"

// FMC SDRAM Modes
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
#define SDRAM_TIMEOUT							 ((uint32_t)0xFFFF)

#define  VSYNC             OTM8009A_480X800_VSYNC
#define  HBP               OTM8009A_480X800_HBP
#define  HFP               OTM8009A_480X800_HFP
#define  HSYNC             OTM8009A_480X800_HSYNC
#define  VBP               OTM8009A_480X800_VBP
#define  VFP               OTM8009A_480X800_VFP
#define VACT 480
#define HACT 800/2

#endif
