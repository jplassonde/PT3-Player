/****************************************************************************
File:     hardware_conf.cpp
Info:     Start up hardware configuration.
		  -System, PLLs & peripheral clock configuration
		  -LTDC, DSI, FMC & SDRAM config
		  -I2C1 and I2C4 for communication with IO expander and touch screen controller
		  -Timers for Sound chip clocks and 20ms signal to player.


The MIT License (MIT)
Copyright (c) 2018 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************/

#include "otm8009a.h"
#include "project.h"
#include "hardware_conf.h"
#include "IrqPriorities.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_audio.h"
#include "Wm8994.h"
#include "usbEvent.h"

extern HCD_HandleTypeDef hhcd;
USBH_HandleTypeDef hUSBHost;
extern USBH_ClassTypeDef G935_Class;


TIM_HandleTypeDef chanATimHandle;
TIM_HandleTypeDef chanBTimHandle;
TIM_HandleTypeDef chanCTimHandle;

I2C_HandleTypeDef ioxI2C;
LTDC_HandleTypeDef  hltdc;
I2C_HandleTypeDef i2c4;
DMA2D_HandleTypeDef hdma2d;
DSI_HandleTypeDef hdsi;
DMA_HandleTypeDef ioxTxDMA;

static void SystemClock_Config();
static void SDRAM_Init();
static void LCD_Init();
static void I2C1_Init();
static void Touch_Init();
static void GPIO_Init();
static void soundTimer_Init();
static void ayClock_Init();
static void toneCounters_Init();

extern void XferCpltCallback(struct __DMA2D_HandleTypeDef * hdma2d);


void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) {
	switch (id) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;
	case HOST_USER_CLASS_ACTIVE:
		break;
	case HOST_USER_CLASS_SELECTED:
		break;
	case HOST_USER_CONNECTION:
		break;
	case HOST_USER_DISCONNECTION:
		break;
	case HOST_USER_UNRECOVERED_ERROR:
		break;
	default:
		break;
	}
}

extern "C" {
void OTG_HS_IRQHandler(void)
{
  HAL_HCD_IRQHandler(&hhcd);
}
}

QueueHandle_t xUsbhQueue;

void UsbTask(void *pvParameters) {
	Usb_Event_t event;
	xUsbhQueue = xQueueCreate(10, sizeof(Usb_Event_t));
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);
	USBH_RegisterClass(&hUSBHost, &G935_Class);
	USBH_Start(&hUSBHost);

	for(;;) {
		xQueueReceive(xUsbhQueue, &event, portMAX_DELAY);
		hUSBHost.e = event;
		USBH_Process(&hUSBHost);
	}
}


void hardware_config() {
	// Enable D&I caches
	SCB_EnableICache();
	SCB_EnableDCache();

	// Enaable ART accelerator and flash prefetch
	__HAL_FLASH_ART_ENABLE();
	__HAL_FLASH_PREFETCH_BUFFER_ENABLE();

	// Set Interrupt Group Priority
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_InitTick(TICK_INT_PRIORITY);

	HAL_Delay(20);
	SystemClock_Config();

	// Enable all GPIO ports clocks
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();

	SDRAM_Init();
	LCD_Init();
	Touch_Init();
	I2C1_Init(); // IO expander
	GPIO_Init();
	soundTimer_Init();
	ayClock_Init();
	toneCounters_Init();

	HAL_Delay(20);
	Wm8994::init();
}

static void SystemClock_Config() {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	// Setup main PLL with HSE as source
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	// Activate OverDrive to reach the 216 MHz Frequency
	HAL_PWREx_EnableOverDrive();

	// Set PLL as clock source, HClock at 216 Mhz, APB1 at 54/108, APB2 at 108/216
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);


	// LCD/SAI1 clock configuration
	// 27.43 MHz to LCD, 48 MHz to SAIP. 25 MHz to SAIQ
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_SDMMC2 | RCC_PERIPHCLK_SAI1;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 7;
	PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
	PeriphClkInitStruct.PLLSAI.PLLSAIQ = 6;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
	PeriphClkInitStruct.PLLSAIDivQ = 1;
	PeriphClkInitStruct.PLLI2S.PLLI2SN = 429;
	PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
	PeriphClkInitStruct.PLLI2SDivQ = 19;
	PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
	PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
	PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLI2S;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

void SDRAM_Init() {
	SDRAM_HandleTypeDef sdramHandle;
	FMC_SDRAM_TimingTypeDef Timing;
	FMC_SDRAM_CommandTypeDef Command;

	sdramHandle.Instance = FMC_Bank5_6;

// Timing configuration for 108Mhz as SDRAM clock frequency (System clock is 216Mhz)
	Timing.LoadToActiveDelay    = 2;
	Timing.ExitSelfRefreshDelay = 7;
	Timing.SelfRefreshTime      = 4;
	Timing.RowCycleDelay        = 7;
	Timing.WriteRecoveryTime    = 2;
	Timing.RPDelay              = 2;
	Timing.RCDDelay             = 2;

// FMC Params
	sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
	sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
	sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
	sdramHandle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
	sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
	sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	sdramHandle.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
	sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
	sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

// Enable FMC clock
	__HAL_RCC_FMC_CLK_ENABLE();

// Init FMC-SDRAM GPIO
	GPIO_InitTypeDef gpio_init_structure;

	gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
	gpio_init_structure.Pull      = GPIO_PULLUP;
	gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
	gpio_init_structure.Alternate = GPIO_AF12_FMC;

	gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |
							  	GPIO_PIN_14 | GPIO_PIN_15;


	HAL_GPIO_Init(GPIOD, &gpio_init_structure);

	gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |
							  	GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
								GPIO_PIN_15;

	HAL_GPIO_Init(GPIOE, &gpio_init_structure);

	gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |
							 	GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
								GPIO_PIN_15;

	HAL_GPIO_Init(GPIOF, &gpio_init_structure);

	gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4|
							  	GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOG, &gpio_init_structure);

	gpio_init_structure.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 |
							  	GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
								GPIO_PIN_15;
	HAL_GPIO_Init(GPIOH, &gpio_init_structure);

	gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
							  	GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOI, &gpio_init_structure);


	HAL_SDRAM_Init(&sdramHandle, &Timing);


// SDRAM Init
	// Clock config enable command
	Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	HAL_Delay(2);

	// Precharge all command */
	Command.CommandMode            = FMC_SDRAM_CMD_PALL;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	//Autorefresh command
	Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 8;
	Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	// External memory mode
	Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = (uint32_t)(SDRAM_MODEREG_BURST_LENGTH_1        |
											  SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
											  SDRAM_MODEREG_CAS_LATENCY_3           |
											  SDRAM_MODEREG_OPERATING_MODE_STANDARD |
											  SDRAM_MODEREG_WRITEBURST_MODE_SINGLE);
	HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

	// Refresh rate adjusted for 108 MHz clk. Should work? The SRAM is overclocked...
	HAL_SDRAM_ProgramRefreshRate(&sdramHandle, 0x67E);
}

static void LCD_Init(){
	DSI_PHY_TimerTypeDef  PhyTimings;
	GPIO_InitTypeDef  gpio_init_structure;

	DSI_PLLInitTypeDef dsiPllInit;
	DSI_CmdCfgTypeDef cmdModeConfig;
	DSI_LPCmdTypeDef LPCmd;

// LCD & TouchScreen Reset pin
	gpio_init_structure.Pin   = GPIO_PIN_15;
	gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
	gpio_init_structure.Pull  = GPIO_PULLUP;
	gpio_init_structure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOJ, &gpio_init_structure);

	// Reset them
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_15, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_Delay(10);


// Start LTDC, DMA2D & DSI clock and do a reset
	__HAL_RCC_LTDC_CLK_ENABLE();
	__HAL_RCC_LTDC_FORCE_RESET();
	__HAL_RCC_LTDC_RELEASE_RESET();
	__HAL_RCC_DMA2D_CLK_ENABLE();
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();
	__HAL_RCC_DSI_CLK_ENABLE();
	__HAL_RCC_DSI_FORCE_RESET();
	__HAL_RCC_DSI_RELEASE_RESET();

// Enable LTDC, DMA2D & DSI interrupts
	NVIC_SetPriority(LTDC_IRQn, LTDC_PRIO);
	NVIC_EnableIRQ(LTDC_IRQn);
	NVIC_SetPriority(DMA2D_IRQn, DMA2D_PRIO);
	NVIC_EnableIRQ(DMA2D_IRQn);
	NVIC_SetPriority(DSI_IRQn, DSI_PRIO);
	NVIC_EnableIRQ(DSI_IRQn);


// Set up DSI
	hdsi.Instance = DSI;

	HAL_DSI_DeInit(&(hdsi));

	dsiPllInit.PLLNDIV  = 100;
	dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV5;
	dsiPllInit.PLLODF   = DSI_PLL_OUT_DIV1;

	hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
	hdsi.Init.TXEscapeCkdiv = 0x4;
	HAL_DSI_Init(&(hdsi), &(dsiPllInit));

	// Config TE Pin
	GPIO_InitTypeDef gpio_TE;

	// Common GPIO configuration
	gpio_TE.Pin = GPIO_PIN_2;
	gpio_TE.Mode      = GPIO_MODE_AF_PP;
	gpio_TE.Pull      = GPIO_NOPULL;
	gpio_TE.Speed     = GPIO_SPEED_FREQ_LOW;
	gpio_TE.Alternate = GPIO_AF13_DSI;
	HAL_GPIO_Init(GPIOJ, &gpio_TE);

	// DSI Command mode config - Triple Checked
	cmdModeConfig.VirtualChannelID      = 0;
	cmdModeConfig.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
	cmdModeConfig.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
	cmdModeConfig.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
	cmdModeConfig.ColorCoding           = DSI_RGB888;
	cmdModeConfig.CommandSize           = 800;
	cmdModeConfig.TearingEffectSource   = DSI_TE_EXTERNAL;
	cmdModeConfig.TearingEffectPolarity = DSI_TE_RISING_EDGE;
	cmdModeConfig.VSyncPol              = DSI_VSYNC_FALLING;
	cmdModeConfig.AutomaticRefresh      = DSI_AR_DISABLE;
	cmdModeConfig.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;

	// Set low power/speed mode for Init
	LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
	LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
	LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
	LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
	LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
	LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
	LPCmd.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
	LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
	LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
	LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
	LPCmd.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;

	// LTDC Config
	hltdc.Init.HorizontalSync = HSYNC-1;
	hltdc.Init.VerticalSync = VSYNC;
	hltdc.Init.AccumulatedHBP = HSYNC+HBP-1;
	hltdc.Init.AccumulatedVBP = VSYNC+VBP-1;
	hltdc.Init.AccumulatedActiveH = VSYNC+VBP+VACT;
	hltdc.Init.AccumulatedActiveW = HSYNC+HBP+HACT-1;
	hltdc.Init.TotalHeigh = VSYNC+VBP+VACT+VFP;
	hltdc.Init.TotalWidth = HSYNC+HBP+HACT+HFP-1;

	// Polarity - Checked
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Green = 0;

	hltdc.Instance = LTDC;

	// DeInit LTDC. Because
	HAL_LTDC_DeInit(&hltdc);

	// Got vals from BSP - Checked, working
	PhyTimings.ClockLaneHS2LPTime = 0x14;
	PhyTimings.ClockLaneLP2HSTime = 0x14;
	PhyTimings.DataLaneHS2LPTime = 0x0A;
	PhyTimings.DataLaneLP2HSTime = 0x0A;
	PhyTimings.DataLaneMaxReadTime = 0;
	PhyTimings.StopWaitTime = 0;

	LTDC_LayerCfgTypeDef Layercfg;

	Layercfg.WindowX0 = 0;
	Layercfg.WindowX1 = 400;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowY1 = 480;
	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	Layercfg.FBStartAdress = 0xC0000000;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	Layercfg.ImageWidth = 800;
	Layercfg.ImageHeight = 480;

	HAL_LTDC_Init(&hltdc);
	HAL_DSI_ConfigAdaptedCommandMode(&hdsi, &cmdModeConfig);
	HAL_DSI_ConfigCommand(&hdsi, &LPCmd);
	HAL_DSI_Start(&(hdsi));

	HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings);
	HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1,
					   OTM8009A_CMD_DISPOFF, 0x00);

	// Send init commands to the display controller
	OTM8009A_Init(OTM8009A_COLMOD_RGB888, OTM8009A_ORIENTATION_LANDSCAPE);

	// Disable the low power/slow DSI mode
	LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
	LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
	LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
	LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
	LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
	LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
	LPCmd.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
	LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
	LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
	LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
	LPCmd.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;

	HAL_DSI_ConfigCommand(&hdsi, &LPCmd);
	HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA);
	HAL_DSI_Refresh(&hdsi);

	// LTDC Init
	HAL_LTDC_ConfigLayer(&hltdc, &Layercfg, 0);

	// DMA2D handle base config
	hdma2d.Instance = DMA2D;
	hdma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
	hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;
	hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[0].InputAlpha = 0xFF;
	hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
	hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;
	hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0xFFFFFFFF;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.XferCpltCallback = XferCpltCallback;
}

static void I2C1_Init() {
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	__HAL_RCC_I2C1_CLK_ENABLE();

	ioxI2C.Instance = I2C1;
	ioxI2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	ioxI2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	ioxI2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	ioxI2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	ioxI2C.Init.OwnAddress1 = 0;
	ioxI2C.Init.OwnAddress2 = 0;
	ioxI2C.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	ioxI2C.Init.Timing = 0xA02B91; // ~1MHz
	ioxI2C.hdmatx = &ioxTxDMA;
	HAL_I2C_Init(&ioxI2C);

	HAL_I2CEx_ConfigAnalogFilter(&ioxI2C, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&ioxI2C, 0);
	HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);

	NVIC_SetPriority(I2C1_EV_IRQn, IOX_I2C_PRIO);
	NVIC_EnableIRQ(I2C1_EV_IRQn);

	// IOX DMA
	__HAL_RCC_DMA1_CLK_ENABLE();

    ioxTxDMA.Instance = DMA1_Stream6;
    ioxTxDMA.Init.Channel = DMA_CHANNEL_1;
    ioxTxDMA.Init.Direction = DMA_MEMORY_TO_PERIPH;
    ioxTxDMA.Init.PeriphInc = DMA_PINC_DISABLE;
    ioxTxDMA.Init.MemInc = DMA_MINC_ENABLE;
    ioxTxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    ioxTxDMA.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    ioxTxDMA.Init.Mode = DMA_NORMAL;
    ioxTxDMA.Init.Priority = DMA_PRIORITY_LOW;
    ioxTxDMA.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    ioxTxDMA.Parent = &ioxI2C;

    NVIC_SetPriority(DMA1_Stream6_IRQn, IOX_DMA_PRIO);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    HAL_DMA_Init(&ioxTxDMA);
}

static void Touch_Init() {
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Alternate = GPIO_AF11_I2C4;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_13, GPIO_PIN_SET);

	NVIC_SetPriority(EXTI15_10_IRQn, TS_INT_PRIO);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	__HAL_RCC_I2C4_CLK_ENABLE();

	i2c4.Instance = I2C4;
	i2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	i2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	i2c4.Init.OwnAddress1 = 0;
	i2c4.Init.OwnAddress2 = 0;
	i2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	i2c4.Init.Timing = 0xA0404E72; // ~100kHz

	HAL_I2C_Init(&i2c4);
	HAL_I2CEx_ConfigAnalogFilter(&i2c4, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&i2c4, 0);

	NVIC_SetPriority(I2C4_EV_IRQn, TS_I2C_PRIO);
	NVIC_EnableIRQ(I2C4_EV_IRQn);
}

static void GPIO_Init() {
	// Config Reset pin and put it in the reset state
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, GPIO_PIN_RESET);
}

// 20ms timer for soundchip tick
static void soundTimer_Init() {
    __HAL_RCC_TIM2_CLK_ENABLE();

	TIM_HandleTypeDef timer;
	timer.Instance = TIM2;
	timer.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
	timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	timer.Init.Prescaler = 8639;
	timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer.Init.Period = 249;
	HAL_TIM_Base_Init(&timer);

	TIM_ClockConfigTypeDef timerClock;
	timerClock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&timer, &timerClock);

	NVIC_SetPriority(TIM2_IRQn, AY_VBLANK_PRIO);
	NVIC_EnableIRQ(TIM2_IRQn);
	HAL_TIM_Base_Start_IT(&timer);
}

static void ayClock_Init() {
	TIM_ClockConfigTypeDef clockCfg;
	TIM_OC_InitTypeDef clockOCCfg;
	TIM_HandleTypeDef tim12Handle;

	GPIO_InitTypeDef GPIO_Init;

	__HAL_RCC_TIM12_CLK_ENABLE();

	GPIO_Init.Pin = GPIO_PIN_6;
	GPIO_Init.Mode = GPIO_MODE_AF_PP;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_Init.Alternate = GPIO_AF9_TIM12;
	HAL_GPIO_Init(GPIOH, &GPIO_Init);

	tim12Handle.Instance = TIM12;
	tim12Handle.Init.Prescaler = 0;
	tim12Handle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
	tim12Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim12Handle.Init.Period = 62 - 1;
	tim12Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	tim12Handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	HAL_TIM_Base_Init(&tim12Handle);

	clockCfg.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&tim12Handle, &clockCfg);
	HAL_TIM_PWM_Init(&tim12Handle);

	clockOCCfg.OCMode = TIM_OCMODE_PWM1;
	clockOCCfg.Pulse = 31;
	clockOCCfg.OCPolarity = TIM_OCPOLARITY_HIGH;
	clockOCCfg.OCFastMode = TIM_OCFAST_ENABLE;
	HAL_TIM_PWM_ConfigChannel(&tim12Handle, &clockOCCfg, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&tim12Handle,TIM_CHANNEL_1);
}


// TIM3 -> AY-3-8913 chan A
// TIM4 -> AY-3-8913 chan B
// TIM7 -> AY-3-8913 chan C
static void toneCounters_Init() {
	TIM_ClockConfigTypeDef clockCfg;
	TIM_HandleTypeDef timHandle;

	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_TIM4_CLK_ENABLE();
	__HAL_RCC_TIM7_CLK_ENABLE();

	timHandle.Instance = TIM3;
	timHandle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
	timHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	timHandle.Init.Prescaler = 496 - 1; // (108M (APB timer clock) / 62 (clock to chip divider) / 8 (internal AY tone counter divider) -1
	timHandle.Init.Period = 0xFFF;
	timHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // reload value not buffered. Changes effective immediately
	clockCfg.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

	HAL_TIM_Base_Init(&timHandle);
	HAL_TIM_ConfigClockSource(&timHandle, &clockCfg);
	__HAL_TIM_ENABLE(&timHandle);

	timHandle.Instance = TIM4;
	HAL_TIM_Base_Init(&timHandle);
	HAL_TIM_ConfigClockSource(&timHandle, &clockCfg);
	__HAL_TIM_ENABLE(&timHandle);

	timHandle.Instance = TIM7;
	HAL_TIM_Base_Init(&timHandle);
	HAL_TIM_ConfigClockSource(&timHandle, &clockCfg);
	__HAL_TIM_ENABLE(&timHandle);

	NVIC_SetPriority(TIM3_IRQn, CHAN_A_TIM_PRIO);
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM4_IRQn, CHAN_B_TIM_PRIO);
	NVIC_EnableIRQ(TIM4_IRQn);

	NVIC_SetPriority(TIM7_IRQn, CHAN_C_TIM_PRIO);
	NVIC_EnableIRQ(TIM7_IRQn);
}
