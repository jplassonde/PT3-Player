/* Most of the Audio setup functions were adapted and corrected from STM's Audio class (usbh_audio.c).
 * Support for HID & a command queue were added to be able to fully utilize the device features.
 * In its current form it is a pile of garbage holding together with cheap duct tape, definitely
 * would need cleanup (dead code omitted/replaced from the original audio class). It works fine though by some miracle.
 */

#include "project.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_audio.h"
#include "usbh_g935.h"
#include "usbEvent.h"
#include "ControlReq.h"
#include "G935_Config.h"
#include <cstring>
#include <cstdlib>

DebugInfo_t dbugInfo;
DebugInfo_t sofInfo;
DebugInfo_t irInfo;
DebugInfo_t ctrlReqInfo;
extern QueueHandle_t xUsbhQueue;

QueueHandle_t xUsbhCtrlQueue;
QueueHandle_t xUsbhHidQueue;

extern uint8_t saiBuffer[882];

uint8_t intInBuffer[32];
uint8_t hidBuffer[32]; // THIS HAS TO CHANGE - TESTING ONLY!

static USBH_StatusTypeDef USBH_G935_InterfaceInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_G935_InterfaceDeInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_G935_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_G935_SOFProcess(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_G935_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef G935_SetVolume (USBH_HandleTypeDef *phost, uint8_t feature, uint8_t channel, uint16_t volume);

static USBH_StatusTypeDef AUDIO_Transmit (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef AUDIO_Control (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef SetControlAttribute (USBH_HandleTypeDef *phost, uint8_t attrib);
static USBH_StatusTypeDef FindHIDControl(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef FindAudioStreamingOUT(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef FindAudioStreamingIN(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef G935_ParseCSDescriptors(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef BuildHeadphonePath(USBH_HandleTypeDef *phost);
static uint32_t FindLinkedUnit(USBH_HandleTypeDef *phost, uint8_t UnitID);
static USBH_StatusTypeDef HandleCSRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef SetEndpointControls(USBH_HandleTypeDef *phost,
                                               uint8_t  Ep,
                                               uint8_t *buff);



static USBH_StatusTypeDef AC_SetCur(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length);

static USBH_StatusTypeDef AC_GetCur(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length);

static USBH_StatusTypeDef AC_GetMin(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length);

static USBH_StatusTypeDef AC_GetMax(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length);

static USBH_StatusTypeDef AC_GetRes(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length);

USBH_ClassTypeDef G935_Class =
{
  "G935",
  0,
  USBH_G935_InterfaceInit,
  USBH_G935_InterfaceDeInit,
  USBH_G935_ClassRequest,
  USBH_G935_Process,
  USBH_G935_SOFProcess,
  NULL,
};

G935_HandleTypeDef GHTD __attribute__ ((section(".dtcmram"))); // Ewww. Used to be dynamically allocated. Hax to fit it in dtcm
G935_HandleTypeDef * G935_Handle;

static bool isocSent = false;
static bool intInDone = false;
static bool intTransfer = false;

static USBH_StatusTypeDef USBH_G935_InterfaceInit(USBH_HandleTypeDef * phost) {

	G935_Handle = &GHTD;
	phost->pActiveClass->pData = G935_Handle;
	USBH_memset(G935_Handle, 0, sizeof(G935_HandleTypeDef));

	FindAudioStreamingIN(phost);
	FindAudioStreamingOUT(phost);

	G935_Handle->headphone.interface = 2;
	G935_Handle->headphone.AltSettings = 2;
	G935_Handle->headphone.Ep = 2;
	G935_Handle->headphone.EpSize = 288;
	G935_Handle->headphone.Poll = 10;
	G935_Handle->headphone.supported = 1;
	G935_Handle->headphone.frequency = 44100;

	uint16_t ep_size_in = 0U;
    for (int index = 0U; index < AUDIO_MAX_AUDIO_STD_INTERFACE; index ++) {
    	if( G935_Handle->stream_in[index].valid == 1U) {
			if(ep_size_in < G935_Handle->stream_in[index].EpSize) {
				ep_size_in = G935_Handle->stream_in[index].EpSize;
				G935_Handle->microphone.interface = G935_Handle->stream_in[index].interface;
				G935_Handle->microphone.AltSettings = G935_Handle->stream_in[index].AltSettings;
				G935_Handle->microphone.Ep = G935_Handle->stream_in[index].Ep;
				G935_Handle->microphone.EpSize = G935_Handle->stream_in[index].EpSize;
				G935_Handle->microphone.Poll = (uint8_t)G935_Handle->stream_out[index].Poll;
				G935_Handle->microphone.supported = 1U;
			}
		}
    }


	if(FindHIDControl(phost) == USBH_OK) {
		G935_Handle->control.supported = 1U;
	}

	/* 3rd Step:  Find and Parse Audio interfaces */
	G935_ParseCSDescriptors (phost);

    /* 4th Step:  Open the Audio streaming pipes*/
	BuildHeadphonePath(phost);
	G935_Handle->headphone.Pipe  = USBH_AllocPipe(phost, G935_Handle->headphone.Ep);
    USBH_OpenPipe(	phost,
    				G935_Handle->headphone.Pipe,
					G935_Handle->headphone.Ep,
					phost->device.address,
					phost->device.speed,
					USB_EP_TYPE_ISOC,
					G935_Handle->headphone.EpSize);

    USBH_LL_SetToggle (phost,  G935_Handle->headphone.Pipe, 0U);


    // Dont bother with mic for now.

    if(G935_Handle->control.supported == 1U) {
    	G935_Handle->control.Pipe  = USBH_AllocPipe(phost, G935_Handle->control.Ep);

    	/* Open pipe for IN endpoint */
		USBH_OpenPipe  (phost,
				G935_Handle->control.Pipe,
				G935_Handle->control.Ep,
					  phost->device.address,
					  phost->device.speed,
					  USB_EP_TYPE_INTR,
					  G935_Handle->control.EpSize);

		USBH_LL_SetToggle (phost,  G935_Handle->control.Pipe, 0U);
    }

    G935_Handle->req_state     = AUDIO_REQ_INIT;
    G935_Handle->control_state = AUDIO_CONTROL_INIT;

    return USBH_OK;
}

static USBH_StatusTypeDef USBH_G935_InterfaceDeInit(USBH_HandleTypeDef *phost) {
	 if(G935_Handle->microphone.Pipe != 0x00U)
	  {
	    USBH_ClosePipe  (phost, G935_Handle->microphone.Pipe);
	    USBH_FreePipe  (phost, G935_Handle->microphone.Pipe);
	    G935_Handle->microphone.Pipe = 0U;     /* Reset the pipe as Free */
	  }

	  if( G935_Handle->headphone.Pipe != 0x00U)
	  {
	    USBH_ClosePipe(phost,  G935_Handle->headphone.Pipe);
	    USBH_FreePipe  (phost,  G935_Handle->headphone.Pipe);
	    G935_Handle->headphone.Pipe = 0U;     /* Reset the pipe as Free */
	  }

	  if( G935_Handle->control.Pipe != 0x00U)
	  {
	    USBH_ClosePipe(phost,  G935_Handle->control.Pipe);
	    USBH_FreePipe  (phost,  G935_Handle->control.Pipe);
	    G935_Handle->control.Pipe = 0U;     /* Reset the pipe as Free */
	  }

	  if(phost->pActiveClass->pData)
	  {
	    USBH_free(phost->pActiveClass->pData);
	    phost->pActiveClass->pData = 0U;
	  }
	  return USBH_OK ;
}

static USBH_StatusTypeDef USBH_G935_ClassRequest(USBH_HandleTypeDef * phost) {
	/* Start with AUDIO REQ state machine */
	USBH_StatusTypeDef status = USBH_BUSY;
	USBH_StatusTypeDef req_status = USBH_BUSY;

	switch (G935_Handle->req_state) {
		case AUDIO_REQ_INIT:
		case AUDIO_REQ_SET_DEFAULT_IN_INTERFACE:
			if(G935_Handle->microphone.supported == 1U) {
				req_status = USBH_SetInterface(phost,
									G935_Handle->microphone.interface,
	                                0U);

				if(req_status == USBH_OK) {
					G935_Handle->req_state = AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE;
				}
			} else {
				G935_Handle->req_state = AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE;
				Usb_Event_t e;
				e.eventType = AUDIO_CLASS_REQ;
				e.eventInfo = 0;
				xQueueSend(xUsbhQueue, &e, 0);
			}
			break;

		case AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE:
		if(G935_Handle->headphone.supported == 1U) {
			req_status = USBH_SetInterface(phost,
								  G935_Handle->headphone.interface,
								  0U);
			if(req_status == USBH_OK) {
				G935_Handle->req_state = AUDIO_REQ_CS_REQUESTS;
				G935_Handle->cs_req_state = AUDIO_REQ_GET_VOLUME;

				G935_Handle->temp_feature  = G935_Handle->headphone.asociated_feature;
				G935_Handle->temp_channels = G935_Handle->headphone.asociated_channels;
				Usb_Event_t e;
				e.eventType = AUDIO_CLASS_REQ;
				e.eventInfo = 0;
				xQueueSend(xUsbhQueue, &e, 0);
			}
		} else {
			G935_Handle->req_state = AUDIO_REQ_CS_REQUESTS;
			G935_Handle->cs_req_state = AUDIO_REQ_GET_VOLUME;
			Usb_Event_t e;
			e.eventType = AUDIO_CLASS_REQ;
			e.eventInfo = 0;
			xQueueSend(xUsbhQueue, &e, 0);
		}
		break;

		case AUDIO_REQ_CS_REQUESTS:
			if(HandleCSRequest (phost) == USBH_OK) {
				G935_Handle->req_state = AUDIO_REQ_SET_IN_INTERFACE;
			}
			break;

		case AUDIO_REQ_SET_IN_INTERFACE:
			if(G935_Handle->microphone.supported == 1U) {
				req_status = USBH_SetInterface(phost,
										  G935_Handle->microphone.interface,
										  G935_Handle->microphone.AltSettings);
				if(req_status == USBH_OK) {
				  G935_Handle->req_state = AUDIO_REQ_SET_OUT_INTERFACE;
				}
			} else {
				G935_Handle->req_state = AUDIO_REQ_SET_OUT_INTERFACE;
				Usb_Event_t e;
				e.eventType = AUDIO_CLASS_REQ;
				e.eventInfo = 0;
				xQueueSend(xUsbhQueue, &e, 0);
			}
			break;

		case AUDIO_REQ_SET_OUT_INTERFACE:
			if(G935_Handle->headphone.supported == 1U) {
				req_status = USBH_SetInterface(phost,
						  G935_Handle->headphone.interface,
						  G935_Handle->headphone.AltSettings);

				if(req_status == USBH_OK) {
				G935_Handle->req_state = AUDIO_REQ_IDLE;
				}
			} else {
				G935_Handle->req_state = AUDIO_REQ_IDLE;
				Usb_Event_t e;
				e.eventType = AUDIO_CLASS_REQ;
				e.eventInfo = 0;
				xQueueSend(xUsbhQueue, &e, 0);
			}
			break;
		case AUDIO_REQ_IDLE:
			G935_Handle->play_state = AUDIO_PLAYBACK_INIT;
			phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
			status  = USBH_OK;
			Usb_Event_t e;
			e.eventType = AUDIO_CLASS_REQ;
			e.eventInfo = 0;
			xQueueSend(xUsbhQueue, &e, 0);
			break;

		default:
			break;
	}
	return status;
}

void processCommand(uint8_t * cmd, uint32_t count) {
	switch(cmd[0]) {
		case 0: {

		}
			break;
		case 4: // LED

			break;

		case 5: // Buttons
			break;
		case 6: // Equalizer
			break;

		case 8: // Battery

			break;

	}
}

void parseHid(USBH_HandleTypeDef *phost, uint8_t * buffer, uint32_t count) {
	switch (buffer[0]) {

	case 1: {// Audio vol control
		SetControlAttribute(phost, buffer[1]);
		ControlReq::Init_t CtrlInit;
		CtrlInit.bmRT = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
		CtrlInit.bReq = UAC_SET_CUR;
		CtrlInit.wVal = 0x200;
		CtrlInit.wIdx = 0x200;
		CtrlInit.wLen = 2;
		CtrlInit.dat = (uint8_t *)&(G935_Handle->headphone.attribute.volume);
		ControlReq * req = new ControlReq(&CtrlInit);
		// Try to put the command on the queue. Abort if queue is full.
		// Alloc'ing just to dealloc immediately is highly moronic though. FIX. queue peek, etc...
		if (xQueueSend(xUsbhCtrlQueue, &req, 0) != pdTRUE) {
			delete req;
		}
		break;
	}

	case 0x11: // Hid Command report
		processCommand(&buffer[2], count-2);
		break;
	}

}

void processHid(USBH_HandleTypeDef *phost) {
	static uint8_t state = 0;
	static uint16_t counter = 0;
	uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;

	switch (state) {
	case 0:
		if (phost->Timer % 10 == 0) {
			irInfo.FRNUM_Start = USBx_HOST->HFNUM & 0xFFFF;
			irInfo.FTREM_Start = (USBx_HOST->HFNUM >> 16) & 0xFFFF;
			irInfo.HFIR = USBx_HOST->HFIR;
			irInfo.HNPTXSTS_Start = USB_OTG_HS->HNPTXSTS;
			irInfo.HPTXSTS_Start = USBx_HOST->HPTXSTS;
			__disable_irq();
			USBH_InterruptReceiveData(phost, intInBuffer,
									 (uint8_t)G935_Handle->control.EpSize,
									 G935_Handle->control.Pipe);
			__enable_irq();
			irInfo.HNPTXSTS_End = USB_OTG_HS->HNPTXSTS;
			irInfo.HPTXSTS_End = USBx_HOST->HPTXSTS;
			irInfo.FRNUM_End = USBx_HOST->HFNUM & 0xFFFF;
			irInfo.FTREM_End = (USBx_HOST->HFNUM >> 16) & 0xFFFF;

			state = 1;
			counter = 0;
		}
		break;
	case 1:
		if (((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->control.Pipe].urb_state == URB_DONE) {
			uint32_t byteCount = ((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->control.Pipe].xfer_count;
			if (byteCount > 0) {
				parseHid(phost, intInBuffer, byteCount);
			}
			state = 0;
		}
	}
}


static USBH_StatusTypeDef G935_AudioProcess(USBH_HandleTypeDef *phost) {
	uint8_t *buff;
	USBH_StatusTypeDef status = USBH_BUSY ;

	switch(G935_Handle->play_state) {
		case AUDIO_PLAYBACK_INIT:
			if( G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->bSamFreqType == 0U) {
				G935_Handle->play_state = AUDIO_PLAYBACK_SET_EP_FREQ;
			} else {
				G935_Handle->play_state = AUDIO_PLAYBACK_SET_EP;
			}
			Usb_Event_t e;
			e.eventType = AUDIO_CLASS_REQ;
			e.eventInfo = 0;
			xQueueSend(xUsbhQueue, &e, 0);
			break;

		case AUDIO_PLAYBACK_SET_EP_FREQ:
			buff = (uint8_t*)G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->tSamFreq[0];
			status = SetEndpointControls(phost, G935_Handle->headphone.Ep, buff);
			if(status == USBH_OK) {
				G935_Handle->play_state = AUDIO_PLAYBACK_IDLE;
			}
			break;

		case AUDIO_PLAYBACK_SET_EP:
			buff = (uint8_t *)(void *)&G935_Handle->headphone.frequency;
			status = SetEndpointControls(phost, G935_Handle->headphone.Ep, buff);
			if(status == USBH_OK) {
				G935_Handle->play_state = AUDIO_SET_G935_CONFIG;
				vTaskDelay(50);
			}
			break;

		case AUDIO_SET_G935_CONFIG: {
			static uint8_t i = 0;
			if (i >= 86 ) {
				AUDIO_Play(phost, saiBuffer, 1764);
				break;
			}

			if (G935_Handle->ctrlReq == nullptr) {
				uint8_t buffer[21] = {0};
				ControlReq::Init_t CtrlInit;
				CtrlInit.bmRT = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
				CtrlInit.bReq = 0x09; // Set report
				CtrlInit.wVal = 0x211; // Output - Report id 0x11
				CtrlInit.wIdx = 3;
				CtrlInit.wLen = 20;
				memcpy(buffer, &g935Commands[i][1], g935Commands[i][0]);
				CtrlInit.dat = buffer;
				G935_Handle->ctrlReq = new ControlReq(&CtrlInit);
			}
			vTaskDelay(20); // Need to put some delay between commands. Esp since its not parsing answers
			if (G935_Handle->ctrlReq != nullptr) {
				if (((ControlReq*)G935_Handle->ctrlReq)->processRequest(phost)) {
					delete (ControlReq*)G935_Handle->ctrlReq;
					G935_Handle->ctrlReq = nullptr;
					++i;
					USBH_InterruptReceiveData(phost, intInBuffer,
											 (uint8_t)G935_Handle->control.EpSize,
											 G935_Handle->control.Pipe);

					G935_Handle->play_state = AUDIO_WAIT_G935_RESP;
				}

			}
		}
			break;

		case AUDIO_WAIT_G935_RESP: {
			if (((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->control.Pipe].urb_state == URB_DONE) {
				G935_Handle->play_state = AUDIO_SET_G935_CONFIG;
			}
		}
			break;

		case AUDIO_PLAYBACK_IDLE:
			status = USBH_OK;
		break;


		case AUDIO_PLAYBACK_PLAY: {
			//if (phost->e.eventType == URB_CHANGE && phost->e.eventInfo == G935_Handle->headphone.Pipe) {
			uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;
			uint32_t txsts = USBx_HOST->HPTXSTS;
			uint8_t queueSpace = (txsts >> 16) & 0xFF;
			if ( queueSpace == 0x08) {
				if (USBH_LL_GetURBState(phost , G935_Handle->headphone.Pipe) == USBH_URB_DONE) {
					AUDIO_Transmit(phost);
				}

				if (isocSent) {
					if (!intInDone) {
						processHid(phost);
						intInDone = true;
					}
				}

				if (G935_Handle->ctrlReq == nullptr && phost->Timer % 5 == 0) {
					xQueueReceive(xUsbhCtrlQueue, &G935_Handle->ctrlReq, 0);

				}
				if (G935_Handle->ctrlReq != nullptr) {
					if (((ControlReq*)G935_Handle->ctrlReq)->processRequest(phost)) {
						delete (ControlReq*)G935_Handle->ctrlReq;
						G935_Handle->ctrlReq = nullptr;
					}
				}
			}

			status = USBH_OK;
			break;
		}
		default:
			break;
	}
	return status;
}


static USBH_StatusTypeDef USBH_G935_Process(USBH_HandleTypeDef *phost) {
	USBH_StatusTypeDef status = USBH_BUSY;

	G935_AudioProcess(phost);

	return status;
}




static USBH_StatusTypeDef USBH_G935_SOFProcess(USBH_HandleTypeDef *phost) {
	if (phost->gState != HOST_CLASS) {
		return USBH_OK;
	}
	uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;
	sofInfo.FRNUM_Start = USBx_HOST->HFNUM & 0xFFFF;
	sofInfo.FTREM_Start = (USBx_HOST->HFNUM >> 16) & 0xFFFF;
	sofInfo.HFIR = USBx_HOST->HFIR;
	sofInfo.HNPTXSTS_Start = USB_OTG_HS->HNPTXSTS;
	sofInfo.HPTXSTS_Start = USBx_HOST->HPTXSTS;
	isocSent = false;
	intInDone = false;
	BaseType_t xHigherPriorityTaskWoken;
	Usb_Event_t e;
	e.eventType = SOF;
	e.eventInfo = 0;
	xQueueSendFromISR(xUsbhQueue, &e, &xHigherPriorityTaskWoken);
	if (xHigherPriorityTaskWoken != pdFALSE) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	return USBH_OK;
}

static USBH_StatusTypeDef FindHIDControl(USBH_HandleTypeDef *phost) {
	uint8_t interface;
	USBH_StatusTypeDef status = USBH_FAIL;

	interface = USBH_FindInterface(phost, AC_CLASS, USB_SUBCLASS_AUDIOCONTROL, 0xFF);
	if(interface == 0xFFU) {
	    return USBH_FAIL;
	}

	for (interface = 0U; interface < USBH_MAX_NUM_INTERFACES; interface++) {
		if((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == 0x03U) && /*HID*/
		   (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0U))
		{
			if((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) == 0x80U) {
				G935_Handle->control.Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
				G935_Handle->control.EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
				G935_Handle->control.interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
				G935_Handle->control.Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
				G935_Handle->control.supported = 1U;
				status = USBH_OK;
				break;
			}
		}
	}
	return status;
}

static USBH_StatusTypeDef ParseCSDescriptors(AUDIO_ClassSpecificDescTypedef *class_desc,
                                      uint8_t ac_subclass,
                                      uint8_t *pdesc) {

	if (ac_subclass == USB_SUBCLASS_AUDIOCONTROL) {
		switch(pdesc[2]) {
		case UAC_HEADER:
			class_desc->cs_desc.HeaderDesc = (AUDIO_HeaderDescTypeDef *)(void *)pdesc;
			break;

		case UAC_INPUT_TERMINAL:
			class_desc->cs_desc.InputTerminalDesc[class_desc->InputTerminalNum++] = (AUDIO_ITDescTypeDef*)(void *)pdesc;
			break;

		case UAC_OUTPUT_TERMINAL:
			class_desc->cs_desc.OutputTerminalDesc[class_desc->OutputTerminalNum++] = (AUDIO_OTDescTypeDef*)(void *)pdesc;
			break;

		case UAC_FEATURE_UNIT:
			class_desc->cs_desc.FeatureUnitDesc[class_desc->FeatureUnitNum++] = (AUDIO_FeatureDescTypeDef*)(void *)pdesc;
			break;

		case UAC_SELECTOR_UNIT:
			class_desc->cs_desc.SelectorUnitDesc[class_desc->SelectorUnitNum++] = (AUDIO_SelectorDescTypeDef*)(void *)pdesc;
			break;

		case UAC_MIXER_UNIT:
			class_desc->cs_desc.MixerUnitDesc[class_desc->MixerUnitNum++] = (AUDIO_MixerDescTypeDef*)(void *)pdesc;
			break;

		default:
			break;
		}
	} else if(ac_subclass == USB_SUBCLASS_AUDIOSTREAMING) {
    	switch(pdesc[2]) {
			case UAC_AS_GENERAL:
				class_desc->as_desc[class_desc->ASNum].GeneralDesc = (AUDIO_ASGeneralDescTypeDef*)(void *)pdesc;
				break;
			case UAC_FORMAT_TYPE:
				class_desc->as_desc[class_desc->ASNum++].FormatTypeDesc = (AUDIO_ASFormatTypeDescTypeDef*)(void *)pdesc;
				break;
			default:
				break;
    	}
    }
	return USBH_OK;
}

static USBH_StatusTypeDef G935_ParseCSDescriptors(USBH_HandleTypeDef *phost) {
	USBH_DescHeader_t            *pdesc;
	uint16_t                      ptr;
	uint8_t                       itf_index = 0U;
	uint8_t                       itf_number = 0U;
	uint8_t                       alt_setting;

	pdesc   = (USBH_DescHeader_t *)(void *)(phost->device.CfgDesc_Raw);
	ptr = USB_LEN_CFG_DESC;

	G935_Handle->class_desc.FeatureUnitNum = 0U;
	G935_Handle->class_desc.InputTerminalNum = 0U;
	G935_Handle->class_desc.OutputTerminalNum = 0U;
	G935_Handle->class_desc.ASNum = 0U;

	while(ptr < phost->device.CfgDesc.wTotalLength) {
		pdesc = USBH_GetNextDesc((uint8_t*)(void *)pdesc, &ptr);

		switch (pdesc->bDescriptorType) {

			case USB_DESC_TYPE_INTERFACE:
				itf_number = *((uint8_t *)(void *)pdesc + 2U);
				alt_setting = *((uint8_t *)(void *)pdesc + 3U);
				itf_index = USBH_FindInterfaceIndex (phost, itf_number, alt_setting);
				break;

			case USB_DESC_TYPE_CS_INTERFACE:
				if(itf_number <= phost->device.CfgDesc.bNumInterfaces) {
					ParseCSDescriptors(&G935_Handle->class_desc,
								   phost->device.CfgDesc.Itf_Desc[itf_index].bInterfaceSubClass,
								   (uint8_t *)(void *)pdesc);
				}
				break;
			default:
				break;
		}
	}
	return USBH_OK;
}

static USBH_StatusTypeDef BuildHeadphonePath(USBH_HandleTypeDef *phost) {
	uint8_t UnitID = 0U, Type, Index;
	uint32_t value;
	uint8_t terminalIndex;
	USBH_StatusTypeDef ret = USBH_OK;

	// Find association between audio streaming and microphone
	for(terminalIndex = 0U; terminalIndex < G935_Handle->class_desc.InputTerminalNum; terminalIndex++) {
		if(LE16(G935_Handle->class_desc.cs_desc.InputTerminalDesc[terminalIndex]->wTerminalType) == 0x101) {
			UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[terminalIndex]->bTerminalID;
			G935_Handle->headphone.asociated_channels =  G935_Handle->class_desc.cs_desc.InputTerminalDesc[terminalIndex]->bNrChannels;
			break;
		}
	}

	for(Index = 0U; Index < G935_Handle->class_desc.ASNum; Index++) {
		if(G935_Handle->class_desc.as_desc[Index].GeneralDesc->bTerminalLink == UnitID) {
			G935_Handle->headphone.asociated_as = Index;
			break;
		}
	}

	do {
		value = FindLinkedUnit(phost, UnitID);
		if (!value) {
		  return USBH_FAIL;
		}

		Index = (uint8_t)(value & 0xFFU);
		Type = (uint8_t)((value >> 8U) & 0xFFU);
		UnitID = (uint8_t)((value >> 16U) & 0xFFU);

		switch (Type) {
			case UAC_FEATURE_UNIT:
				G935_Handle->headphone.asociated_feature = Index;
			  break;

			case UAC_MIXER_UNIT:
				G935_Handle->headphone.asociated_mixer = Index;
			  break;

			case UAC_SELECTOR_UNIT:
				G935_Handle->headphone.asociated_selector = Index;
			  break;

			case UAC_OUTPUT_TERMINAL:
				G935_Handle->headphone.asociated_terminal = Index;
				if(LE16(G935_Handle->class_desc.cs_desc.OutputTerminalDesc[Index]->wTerminalType) != 0x103) {
					return  USBH_OK;
				}
				break;

			default:
				ret = USBH_FAIL;
				break;
		}
	} while ((Type != UAC_OUTPUT_TERMINAL) && (value > 0U));
  return ret;
}

static USBH_StatusTypeDef FindAudioStreamingIN(USBH_HandleTypeDef *phost) {
	uint8_t interface, alt_settings;
	USBH_StatusTypeDef status = USBH_FAIL ;

	/* Look For AUDIOSTREAMING IN interface */
	alt_settings = 0U;
	for (interface = 0U; interface < USBH_MAX_NUM_INTERFACES; interface++) {
		if((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == AC_CLASS)&&
		   (phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING))
		{
			if((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) &&
				(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0U))
			{
				G935_Handle->stream_in[alt_settings].Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
				G935_Handle->stream_in[alt_settings].EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
				G935_Handle->stream_in[alt_settings].interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
				G935_Handle->stream_in[alt_settings].AltSettings = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
				G935_Handle->stream_in[alt_settings].Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
				G935_Handle->stream_in[alt_settings].valid = 1U;
				alt_settings++;
			}
		}
	}

	if(alt_settings > 0U) {
		status = USBH_OK;
	}
	return status;
}

/**
  * @brief  Find OUT Audio Streaming interfaces
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef FindAudioStreamingOUT(USBH_HandleTypeDef *phost) {
	uint8_t interface, alt_settings;
	USBH_StatusTypeDef status = USBH_FAIL ;

	/* Look For AUDIOSTREAMING OUT interface */
	alt_settings = 0U;
	for (interface = 0U; interface < USBH_MAX_NUM_INTERFACES; interface++) {
		if((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == AC_CLASS)&&
			(phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING))
		{
			if(((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) == 0x00U) &&
				(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0U))
			{
				G935_Handle->stream_out[alt_settings].Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
				G935_Handle->stream_out[alt_settings].EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
				G935_Handle->stream_out[alt_settings].interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
				G935_Handle->stream_out[alt_settings].AltSettings = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
				G935_Handle->stream_out[alt_settings].Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
				G935_Handle->stream_out[alt_settings].valid = 1U;
				alt_settings++;
			}
		}
	}

	if(alt_settings > 0U) {
		status = USBH_OK;
	}
	return status;
}

static uint32_t FindLinkedUnit(USBH_HandleTypeDef *phost, uint8_t UnitID) {
	uint8_t Index;

	/* Find Feature Unit */
	for(Index = 0U; Index < G935_Handle->class_desc.FeatureUnitNum; Index ++) {
		if(G935_Handle->class_desc.cs_desc.FeatureUnitDesc[Index]->bSourceID == UnitID) {
			UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[Index]->bUnitID;
			return (((uint32_t)UnitID << 16U) | (UAC_FEATURE_UNIT << 8U) | (uint32_t)Index);
		}
	}

	/* Find Mixer Unit */
	for(Index = 0U; Index < G935_Handle->class_desc.MixerUnitNum; Index ++) {
		if((G935_Handle->class_desc.cs_desc.MixerUnitDesc[Index]->bSourceID0 == UnitID)||
		   (G935_Handle->class_desc.cs_desc.MixerUnitDesc[Index]->bSourceID1 == UnitID))
		{
			UnitID = G935_Handle->class_desc.cs_desc.MixerUnitDesc[Index]->bUnitID;

			return ((UnitID << 16U) | (UAC_MIXER_UNIT << 8U) | Index);
		}
	}

	/* Find Selector Unit */
	for(Index = 0U; Index < G935_Handle->class_desc.SelectorUnitNum; Index ++) {
		if(G935_Handle->class_desc.cs_desc.SelectorUnitDesc[Index]->bSourceID0 == UnitID) {
			UnitID = G935_Handle->class_desc.cs_desc.SelectorUnitDesc[Index]->bUnitID;
			return ((UnitID << 16U) | (UAC_SELECTOR_UNIT << 8U) | Index);
		}
	}

	/* Find OT Unit */
	for(Index = 0U; Index < G935_Handle->class_desc.OutputTerminalNum; Index ++) {
		if(G935_Handle->class_desc.cs_desc.OutputTerminalDesc[Index]->bSourceID == UnitID) {
			UnitID = G935_Handle->class_desc.cs_desc.OutputTerminalDesc[Index]->bTerminalID;
			return ((UnitID << 16U) | (UAC_OUTPUT_TERMINAL << 8U) | Index);
		}
	}
  /* No associated Unit found return undefined ID 0x00*/
  return 0U;
}

static USBH_StatusTypeDef AC_SetCur(USBH_HandleTypeDef *phost,
											 uint8_t subtype,
											 uint8_t feature,
											 uint8_t controlSelector,
											 uint8_t channel,
											 uint16_t length)
{
	uint16_t wValue,wIndex,wLength;
	uint8_t UnitID,InterfaceNum;
	USBH_StatusTypeDef ret = USBH_OK;

	switch(subtype) {
		case UAC_INPUT_TERMINAL:
			UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[0]->bTerminalID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			wValue = (COPY_PROTECT_CONTROL << 8U);
			G935_Handle->mem[0] = 0x00U;

			wLength = 1U;
			break;

		case UAC_FEATURE_UNIT:
			UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[feature]->bUnitID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum ;
			/*holds the CS(control selector ) and CN (channel number)*/
			wValue =  (uint16_t)((uint32_t)controlSelector << 8U) | (uint16_t)channel;
			wLength = length;
			break;

		default:
			ret = USBH_FAIL;
			break;
	}

	if (ret != USBH_OK) {
		return ret;
	}

	phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE
												   | USB_REQ_TYPE_CLASS;

	phost->Control.setup.b.bRequest = UAC_SET_CUR;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	return(USBH_CtlReq(phost, (uint8_t *)(void *)(G935_Handle->mem), wLength));
}

static USBH_StatusTypeDef AC_GetCur(USBH_HandleTypeDef *phost,
                                         uint8_t subtype,
                                         uint8_t feature,
                                         uint8_t controlSelector,
                                         uint8_t channel,
                                         uint16_t length)
{
	uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;
	uint8_t UnitID = 0U, InterfaceNum = 0U;
	USBH_StatusTypeDef ret = USBH_OK;

	switch(subtype) {
		case UAC_INPUT_TERMINAL:
			UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[0]->bTerminalID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			wValue = (COPY_PROTECT_CONTROL << 8U);
			G935_Handle->mem[0] = 0x00U;

			wLength = 1U;
			break;
		case UAC_FEATURE_UNIT:
			UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[feature]->bUnitID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			/*holds the CS(control selector ) and CN (channel number)*/
			wValue =  (uint16_t)((uint32_t)controlSelector << 8U) | (uint16_t)channel;
			wLength = length;
			break;

		case UAC_OUTPUT_TERMINAL:
			UnitID = G935_Handle->class_desc.cs_desc.OutputTerminalDesc[0]->bTerminalID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			wValue = (COPY_PROTECT_CONTROL << 8U);
			wLength = 1U;
			break;

		default:
			ret = USBH_FAIL;
			break;
	}

	if (ret != USBH_OK) {
		return ret;
	}

	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | \
	USB_REQ_TYPE_CLASS;

	phost->Control.setup.b.bRequest = UAC_GET_CUR;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	return(USBH_CtlReq(phost, (uint8_t *)(void *)(G935_Handle->mem) , wLength ));
}

static USBH_StatusTypeDef AC_GetMax(USBH_HandleTypeDef *phost,
                                         uint8_t subtype,
                                         uint8_t feature,
                                         uint8_t controlSelector,
                                         uint8_t channel,
                                         uint16_t length)
{
  uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;
  uint8_t UnitID = 0U, InterfaceNum = 0U;
  G935_HandleTypeDef *G935_Handle;
  G935_Handle =  (G935_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_StatusTypeDef ret = USBH_OK;

  switch(subtype)
  {
  case UAC_INPUT_TERMINAL:
    UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[0]->bTerminalID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
    wValue = (COPY_PROTECT_CONTROL << 8U);
    G935_Handle->mem[0] = 0x00U;

    wLength = 1U;
    break;
  case UAC_FEATURE_UNIT:
    UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[feature]->bUnitID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum ;
    /*holds the CS(control selector ) and CN (channel number)*/
    wValue =  (uint16_t)((uint32_t)controlSelector << 8U) | (uint16_t)channel;
    wLength = length;
    break;

  case UAC_OUTPUT_TERMINAL:
    UnitID = G935_Handle->class_desc.cs_desc.OutputTerminalDesc[0]->bTerminalID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum ;
    wValue = (COPY_PROTECT_CONTROL << 8U) ;
    wLength = 1U;
    break;

  default:
    ret = USBH_FAIL;
    break;
  }

  if (ret != USBH_OK)
  {
    return ret;
  }

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | \
    USB_REQ_TYPE_CLASS;

  phost->Control.setup.b.bRequest = UAC_GET_MAX;
  phost->Control.setup.b.wValue.w = wValue;
  phost->Control.setup.b.wIndex.w = wIndex;
  phost->Control.setup.b.wLength.w = wLength;

  return(USBH_CtlReq(phost, (uint8_t *)(void *)(G935_Handle->mem), wLength));

}

/**
  * @brief  Handle Get Res request
  * @param  phost: Host handle
  * @param  subtype: subtype index
  * @param  feature: feature index
  * @param  controlSelector: control code
  * @param  channel: channel index
  * @param  length: Command length
  * @retval USBH Status
  */
static USBH_StatusTypeDef AC_GetRes(USBH_HandleTypeDef *phost,
                                         uint8_t subtype,
                                         uint8_t feature,
                                         uint8_t controlSelector,
                                         uint8_t channel,
                                         uint16_t length)
{
  uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;
  uint8_t UnitID = 0U, InterfaceNum = 0U;
  G935_HandleTypeDef *G935_Handle;
  G935_Handle =  (G935_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_StatusTypeDef ret = USBH_OK;

  switch(subtype)
  {
  case UAC_INPUT_TERMINAL:
    UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[0]->bTerminalID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
    wValue = (COPY_PROTECT_CONTROL << 8U) ;
    G935_Handle->mem[0] = 0x00U;

    wLength = 1U;
    break;
  case UAC_FEATURE_UNIT:
    UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[feature]->bUnitID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
    /*holds the CS(control selector ) and CN (channel number)*/
    wValue =  (uint16_t)((uint32_t)controlSelector << 8U) | (uint16_t)channel;
    wLength = length;
    break;

  case UAC_OUTPUT_TERMINAL:
    UnitID = G935_Handle->class_desc.cs_desc.OutputTerminalDesc[0]->bTerminalID;
    InterfaceNum = 0U; /*Always zero Control Interface */
    wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
    wValue = (COPY_PROTECT_CONTROL << 8U) ;
    wLength = 1U;
    break;

  default:
    ret = USBH_FAIL;
    break;
  }

  if (ret != USBH_OK)
  {
    return ret;
  }

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE
                                                 | USB_REQ_TYPE_CLASS;

  phost->Control.setup.b.bRequest = UAC_GET_RES;
  phost->Control.setup.b.wValue.w = wValue;
  phost->Control.setup.b.wIndex.w = wIndex;
  phost->Control.setup.b.wLength.w = wLength;

  return(USBH_CtlReq(phost, (uint8_t *)(void *)(G935_Handle->mem), wLength));

}

/**
  * @brief  Handle Get Min request
  * @param  phost: Host handle
  * @param  subtype: subtype index
  * @param  feature: feature index
  * @param  controlSelector: control code
  * @param  channel: channel index
  * @param  length: Command length
  * @retval USBH Status
  */
static USBH_StatusTypeDef AC_GetMin(USBH_HandleTypeDef *phost,
                                         uint8_t subtype,
                                         uint8_t feature,
                                         uint8_t controlSelector,
                                         uint8_t channel,
                                         uint16_t length)
{
	uint16_t wValue = 0U, wIndex = 0U, wLength = 0U;
	uint8_t UnitID = 0U, InterfaceNum = 0U;
	USBH_StatusTypeDef ret = USBH_OK;

	switch(subtype) {
		case UAC_INPUT_TERMINAL:
			UnitID = G935_Handle->class_desc.cs_desc.InputTerminalDesc[0]->bTerminalID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			wValue = (COPY_PROTECT_CONTROL << 8U);
			G935_Handle->mem[0] = 0x00U;

			wLength = 1U;
			break;
		case UAC_FEATURE_UNIT:
			UnitID = G935_Handle->class_desc.cs_desc.FeatureUnitDesc[feature]->bUnitID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			/*holds the CS(control selector ) and CN (channel number)*/
			wValue =  (uint16_t)((uint32_t)controlSelector << 8U) | (uint16_t)channel;
			wLength = length;
			break;

		case UAC_OUTPUT_TERMINAL:
			UnitID = G935_Handle->class_desc.cs_desc.OutputTerminalDesc[0]->bTerminalID;
			InterfaceNum = 0U; /*Always zero Control Interface */
			wIndex = (uint16_t)((uint32_t)UnitID << 8U) | (uint16_t)InterfaceNum;
			wValue = (COPY_PROTECT_CONTROL << 8U);
			wLength = 1U;
			break;

		default:
			ret = USBH_FAIL;
			break;
	}

	if (ret != USBH_OK) {
		return ret;
	}

	phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | \
	USB_REQ_TYPE_CLASS;

	phost->Control.setup.b.bRequest = UAC_GET_MIN;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	return(USBH_CtlReq(phost, (uint8_t *)(void *)(G935_Handle->mem), wLength));
}

static USBH_StatusTypeDef CSRequest(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t channel)
{
	USBH_StatusTypeDef status = USBH_BUSY;
	USBH_StatusTypeDef req_status = USBH_BUSY;
	uint16_t VolumeCtl, ResolutionCtl;
	// Why are they using channel / from where did they get that idea / why is this wrong?
	// Logitech headphones dont do this by the standard or is it a mistake?
	// Just clear it to 0 here so we dont get STALLs and get the data we need
	channel = 0;

	/* Switch AUDIO REQ state machine */
	switch (G935_Handle->cs_req_state) {
		case AUDIO_REQ_GET_VOLUME:
			req_status = AC_GetCur(phost,
									UAC_FEATURE_UNIT,     /* subtype  */
									feature,              /* feature  */
									VOLUME_CONTROL,       /* Selector */
									channel,              /* channel  */
									0x02U);               /* length   */
			if(req_status != USBH_BUSY) {
				G935_Handle->cs_req_state = AUDIO_REQ_GET_MIN_VOLUME;
				// This is discarding the correct value and
				// collecting the 16 bit of random garbage just after..
				// VolumeCtl = LE16(&(G935_Handle->mem[0]));
				VolumeCtl = G935_Handle->mem[0]; // The data is already in the right format
				G935_Handle->headphone.attribute.volume = (uint32_t)VolumeCtl;
			}
			break;

		case AUDIO_REQ_GET_MIN_VOLUME:
			req_status = AC_GetMin(phost,
									UAC_FEATURE_UNIT,     /* subtype  */
									feature,              /* feature  */
									VOLUME_CONTROL,       /* Selector */
									channel,              /* channel  */
									0x02U);               /* length   */
			if(req_status != USBH_BUSY)	{
				G935_Handle->cs_req_state = AUDIO_REQ_GET_MAX_VOLUME;
				VolumeCtl = G935_Handle->mem[0];
				G935_Handle->headphone.attribute.volumeMin = (uint32_t)VolumeCtl;
			}
			break;

		case AUDIO_REQ_GET_MAX_VOLUME:
			req_status = AC_GetMax(phost,
									UAC_FEATURE_UNIT,     /* subtype  */
									feature,              /* feature  */
									VOLUME_CONTROL,       /* Selector */
									channel,              /* channel  */
									0x02U);               /* length   */
			if(req_status != USBH_BUSY) {
				G935_Handle->cs_req_state = AUDIO_REQ_GET_RESOLUTION;
				VolumeCtl = G935_Handle->mem[0];
				G935_Handle->headphone.attribute.volumeMax = (uint32_t)VolumeCtl;

				/* This sets the maximum to like 90% no thanks, i need to blast them bleeps.
				if (G935_Handle->headphone.attribute.volumeMax < G935_Handle->headphone.attribute.volumeMin) {
					G935_Handle->headphone.attribute.volumeMax = 0xFF00U;
				} */
			}
			break;

		case AUDIO_REQ_GET_RESOLUTION:
			req_status = AC_GetRes(phost,
									UAC_FEATURE_UNIT,     /* subtype  */
									feature,              /* feature  */
									VOLUME_CONTROL,       /* Selector */
									channel,              /* channel  */
									0x02U);                /* length   */
			if(req_status != USBH_BUSY) {
				G935_Handle->cs_req_state = AUDIO_REQ_CS_IDLE;
				ResolutionCtl = G935_Handle->mem[0];
				G935_Handle->headphone.attribute.resolution = (uint32_t)ResolutionCtl;
			}
			break;

		case AUDIO_REQ_CS_IDLE:
			status = USBH_OK;
			break;

		default:
			break;
	}
	return status;
}

USBH_StatusTypeDef G935_SetFrequency (USBH_HandleTypeDef *phost,
                                            uint16_t SampleRate,
                                            uint8_t  NbrChannels,
                                            uint8_t  BitPerSample)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  uint8_t              index;
  uint8_t              change_freq = FALSE;
  uint32_t             freq_min, freq_max;
  uint8_t              num_supported_freq;

  if(phost->gState == HOST_CLASS)
  {
    if(G935_Handle->play_state == AUDIO_PLAYBACK_IDLE)
    {

      if(G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->bSamFreqType == 0U)
      {
        freq_min = LE24(G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->tSamFreq[0]);
        freq_max = LE24(G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->tSamFreq[1]);

        if(( SampleRate >= freq_min)&& (SampleRate <= freq_max))
        {
          change_freq = TRUE;
        }
      }
      else
      {

        num_supported_freq = (G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->bLength - 8U) / 3U;

        for(index = 0U; index < num_supported_freq; index++)
        {
          if(SampleRate == LE24(G935_Handle->class_desc.as_desc[G935_Handle->headphone.asociated_as].FormatTypeDesc->tSamFreq[index]))
          {
            change_freq = TRUE;
            break;
          }
        }
      }

      if(change_freq == TRUE) {
    	  G935_Handle->headphone.frequency = SampleRate;
    	  G935_Handle->headphone.frame_length = (SampleRate * BitPerSample * NbrChannels) / 8000U;
    	  G935_Handle->play_state = AUDIO_PLAYBACK_SET_EP;

    	  Usb_Event_t e;
    	  e.eventType = AUDIO_CLASS_REQ;
    	  e.eventInfo = 0;
    	  xQueueSend(xUsbhQueue, &e, 0);

    	  Status = USBH_OK;

      }
    }
  }
   return Status;
}

static USBH_StatusTypeDef HandleCSRequest(USBH_HandleTypeDef *phost) {
	USBH_StatusTypeDef status = USBH_BUSY;
	USBH_StatusTypeDef cs_status = USBH_BUSY;

	cs_status = CSRequest(phost, G935_Handle->temp_feature,
								 G935_Handle->temp_channels);

	if(cs_status != USBH_BUSY) {
		if(G935_Handle->temp_channels == 1U) {
			G935_Handle->temp_feature = G935_Handle->headphone.asociated_feature;
			G935_Handle->temp_channels = 0U;
			status = USBH_OK;
		} else {
			G935_Handle->temp_channels--;
		}
		G935_Handle->cs_req_state = AUDIO_REQ_GET_VOLUME;
		Usb_Event_t e;
		e.eventType = USBH_URB_EVENT;
		e.eventInfo = 0;
		xQueueSend(xUsbhQueue, &e, 0);
	}

	return status;
}

static USBH_StatusTypeDef SetEndpointControls(USBH_HandleTypeDef *phost,
														   uint8_t  Ep,
														   uint8_t *buff)
{
	uint16_t wValue, wIndex, wLength;

	wValue = SAMPLING_FREQ_CONTROL << 8U;
	wIndex = Ep;
	wLength = 3U; /*length of the frequency parameter*/

	phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_ENDPOINT | \
										   USB_REQ_TYPE_CLASS;

	phost->Control.setup.b.bRequest = UAC_SET_CUR;
	phost->Control.setup.b.wValue.w = wValue;
	phost->Control.setup.b.wIndex.w = wIndex;
	phost->Control.setup.b.wLength.w = wLength;

	return(USBH_CtlReq(phost, (uint8_t *)buff, wLength));
}

static USBH_StatusTypeDef AUDIO_Transmit (USBH_HandleTypeDef *phost) {
	USBH_StatusTypeDef status = USBH_BUSY ;
	uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;

	G935_Handle->headphone.total_length = 1764;

	if(G935_Handle->headphone.global_ptr >= G935_Handle->headphone.total_length) {
		G935_Handle->headphone.global_ptr = 0;
		G935_Handle->headphone.cbuf = G935_Handle->headphone.buf;
	}

	// Send 9 * 176 bytes frames then a 180 bytes frame. 1764 = 10ms of 44100Hz sampling rate * 16bits * 2chan
	if (G935_Handle->headphone.global_ptr == 1584) {
		G935_Handle->headphone.frame_length = 180;
	} else {
		G935_Handle->headphone.frame_length = 176;
	}

	/* The frequency if the SAI clock is not exactly 44100 (44099.something). Both transfer eventually
	 * clash together (after a while). To avoid massive audio glitches the following is necessary.
	 */

	// Check where the Sai Rx DMA is at. Total buffer size - data unit remaining (16 bit each)
	// Volatile to prevent optimize out for debug
	volatile uint16_t rxBuffPos = 1764 - (DMA2_Stream4->NDTR) * 2;

	if (rxBuffPos >= G935_Handle->headphone.global_ptr
		&& rxBuffPos <= G935_Handle->headphone.global_ptr+G935_Handle->headphone.frame_length) {
		isocSent = true; // This is a lie but its all good. Let hid int IN/OUT or ctrl do work during that frame.
		return status;
	}

	// Dont bother queueing a transfer if there isn't enough time in the current frame to finish the operation,
	// that would just be asking for trouble. Most likely only happens at startup, still worth the check.
	if (((USBx_HOST->HFNUM >> 16) & 0xFFFF) < 200) {
		return status; // Do not check commands/HID. Wait for SOF to do stuff.
	}

	// Need to find a better fix here. Works for now
	// IF the task get interrupted during the transfer, with the amount of busy-wait loops in STM32
	// ISRs (DMAs and SDMMC being 2 culprits) the Isoc xfer risk being incomplete at the EOFrame
	// Incomplete isoc = a few interrupts that result in a channel halt, fifo clear, etc...
	// AND a URB status set back to URB_READY -BEFORE- this funct set it to URB idle
	// = This function will never be called again.
	__disable_irq();

	// Save relevant registers in debug structure for... debug.
	dbugInfo.FRNUM_Start = USBx_HOST->HFNUM & 0xFFFF;
	dbugInfo.FTREM_Start = (USBx_HOST->HFNUM >> 16) & 0xFFFF;
	dbugInfo.HFIR = USBx_HOST->HFIR;
	dbugInfo.HNPTXSTS_Start = USB_OTG_HS->HNPTXSTS;
	dbugInfo.HPTXSTS_Start = USBx_HOST->HPTXSTS;

	while ((USBx_HOST->HPTXSTS >> 16) & 0xFF != 8) {

	}
	// There will ALWAYS be enough space in fifo since only 1 transac is queued per frame. (and possibly 1 small one for HID int IN/OUT
	// Don't bother checking, live dangerously.

	// Set xfersize to packet length, set packet count to 1
	USBx_HC(2)->HCTSIZ = ((G935_Handle->headphone.frame_length & USB_OTG_HCTSIZ_XFRSIZ) | ((1 << 19) & USB_OTG_HCTSIZ_PKTCNT));

	// Setup channel 2 for the transfer. non-disabled, output, enabled, schedule for next frame.
	uint32_t hcchar = USBx_HC(2)->HCCHAR;
	hcchar &= ~USB_OTG_HCCHAR_CHDIS & ~USB_OTG_HCCHAR_EPDIR;
	hcchar |= USB_OTG_HCCHAR_CHENA;

	if (USBx_HOST->HFNUM & 0x01) {
		hcchar &= ~USB_OTG_HCCHAR_ODDFRM;
	} else {
		hcchar |= USB_OTG_HCCHAR_ODDFRM;
	}
	USBx_HC(2)->HCCHAR = hcchar;

	// Copy the frame with 32bit words to the channel fifo manually.
	for (uint8_t i = 0; i < G935_Handle->headphone.frame_length/4; i++)
	    {
	      USBx_DFIFO(2) = ((uint32_t *)G935_Handle->headphone.cbuf)[i];
	    }

	dbugInfo.HNPTXSTS_End = USB_OTG_HS->HNPTXSTS;
	dbugInfo.HPTXSTS_End = USBx_HOST->HPTXSTS;
	dbugInfo.FRNUM_End = USBx_HOST->HFNUM & 0xFFFF;
	dbugInfo.FTREM_End = (USBx_HOST->HFNUM >> 16) & 0xFFFF;

	G935_Handle->headphone.cbuf += G935_Handle->headphone.frame_length;
	G935_Handle->headphone.global_ptr += G935_Handle->headphone.frame_length;
	((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->headphone.Pipe].urb_state = URB_IDLE;

	__enable_irq();
	isocSent = true;

	return status;
}


USBH_StatusTypeDef AUDIO_Play (USBH_HandleTypeDef *phost, uint8_t *buf, uint32_t length) {
	USBH_StatusTypeDef Status = USBH_OK;
	G935_Handle->headphone.buf = buf;
	G935_Handle->headphone.total_length = length;
	G935_Handle->control_state = AUDIO_CONTROL_INIT;
	G935_Handle->processing_state = AUDIO_DATA_START_OUT;
	G935_Handle->headphone.partial_ptr = 0;
	G935_Handle->headphone.global_ptr = 0;
	G935_Handle->headphone.cbuf = G935_Handle->headphone.buf;
	G935_Handle->play_state = AUDIO_PLAYBACK_PLAY;
	// Not a hax. Definitely not.
	((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->headphone.Pipe].urb_state = URB_DONE;
	((HCD_HandleTypeDef*)(phost->pData))->hc[G935_Handle->control.Pipe].urb_state = URB_DONE;

	return Status;
}

static USBH_StatusTypeDef SetControlAttribute (USBH_HandleTypeDef *phost, uint8_t attrib) {
	switch (attrib) {
		case 0x01:
			if (G935_Handle->headphone.attribute.volume != 0) {// max vol
				G935_Handle->headphone.attribute.volume += G935_Handle->headphone.attribute.resolution;
			}
				break;

		case 0x02:
		// Pro stuff right there
			G935_Handle->headphone.attribute.volume = ((0x10000 | G935_Handle->headphone.attribute.volume)
												- G935_Handle->headphone.attribute.resolution) & 0xFFFF;
			break;
		default :
			break;
	}

	if(G935_Handle->headphone.attribute.volume > 0xFFFF) {
		G935_Handle->headphone.attribute.volume = G935_Handle->headphone.attribute.volumeMax;
	}

	if(G935_Handle->headphone.attribute.volume < G935_Handle->headphone.attribute.volumeMin && G935_Handle->headphone.attribute.volume != 0) {
		G935_Handle->headphone.attribute.volume = G935_Handle->headphone.attribute.volumeMin;
	}
	return USBH_OK;
}

USBH_StatusTypeDef G935_ChangeVolume (USBH_HandleTypeDef *phost, AUDIO_VolumeCtrlTypeDef volume_ctl) {

	if((volume_ctl == VOLUME_UP) || (volume_ctl == VOLUME_DOWN)) {
		if(phost->gState == HOST_CLASS) {
			if(G935_Handle->play_state == AUDIO_PLAYBACK_PLAY) {
				G935_Handle->control_state = (volume_ctl == VOLUME_UP)? AUDIO_CONTROL_VOLUME_UP : AUDIO_CONTROL_VOLUME_DOWN;
				return USBH_OK;
			}
		}
	}

	return USBH_FAIL;
}


static USBH_StatusTypeDef AUDIO_Control (USBH_HandleTypeDef *phost) {
	USBH_StatusTypeDef status = USBH_BUSY ;
	uint16_t attribute  = 0U;

	switch(G935_Handle->control_state) {
		case AUDIO_CONTROL_INIT:
			if((phost->Timer & 1U) == 0U) {
				G935_Handle->control.timer = phost->Timer;
				USBH_InterruptReceiveData(phost,
										(uint8_t *)(void *)(G935_Handle->mem),
										(uint8_t)G935_Handle->control.EpSize,
										G935_Handle->control.Pipe);

				G935_Handle->temp_feature  = G935_Handle->headphone.asociated_feature;
				G935_Handle->temp_channels = 0;

				G935_Handle->control_state = AUDIO_CONTROL_CHANGE ;
			}
			break;

		case AUDIO_CONTROL_CHANGE:
			if(USBH_LL_GetURBState(phost , G935_Handle->control.Pipe) == USBH_URB_DONE) {
				attribute = LE16(&G935_Handle->mem[0]);
				if (SetControlAttribute (phost, (uint8_t)attribute) == USBH_BUSY) {
					break;
				}
			}

			if(( phost->Timer - G935_Handle->control.timer) >= G935_Handle->control.Poll) {
				G935_Handle->control.timer = phost->Timer;

				USBH_InterruptReceiveData(phost,
										(uint8_t *)(void *)(G935_Handle->mem),
										(uint8_t)G935_Handle->control.EpSize,
										G935_Handle->control.Pipe);

			}
			break;

		case AUDIO_CONTROL_VOLUME_UP:
			if(SetControlAttribute (phost, 1U) == USBH_OK) {
				G935_Handle->control_state = AUDIO_CONTROL_INIT;
				status = USBH_OK;
			}
			break;

		case AUDIO_CONTROL_VOLUME_DOWN:
			if( SetControlAttribute (phost, 2U) == USBH_OK) {
				G935_Handle->control_state = AUDIO_CONTROL_INIT;
				status = USBH_OK;
			}
			break;

		case AUDIO_CONTROL_IDLE:
		default:
			break;
  }

  return status;
}


static USBH_StatusTypeDef G935_SetVolume (USBH_HandleTypeDef *phost, uint8_t feature, uint8_t channel, uint16_t volume) {
	USBH_StatusTypeDef status = USBH_BUSY ;
	G935_Handle->mem[0] = volume;

	status = AC_SetCur(phost, UAC_FEATURE_UNIT, feature,
						  VOLUME_CONTROL, channel, 2U);

	return status;
}


/****************************************************
 * Functions called from the user program.
 ****************************************************/

void G935_SetVolume(uint8_t vol) {
	uint16_t headsetVol;
	ControlReq::Init_t CtrlInit;

	if (vol == 100) {
		headsetVol = 0;
	} else {
		headsetVol = ((0x10000-G935_Handle->headphone.attribute.volumeMin) * vol / 100) + G935_Handle->headphone.attribute.volumeMin;
	}

	if (abs((int32_t)(G935_Handle->headphone.attribute.volume) - (int32_t)headsetVol) < 100) {
		return;
	}

	G935_Handle->headphone.attribute.volume = headsetVol;

	CtrlInit.bmRT = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
	CtrlInit.bReq = UAC_SET_CUR;
	CtrlInit.wVal = 0x200;
	CtrlInit.wIdx = 0x200;
	CtrlInit.wLen = 2;
	CtrlInit.dat = (uint8_t *)&headsetVol;

	ControlReq * req = new ControlReq(&CtrlInit);
	// Try to put the command on the queue. Abort if queue is full.
	// Alloc'ing just to dealloc immediately is highly moronic though. FIX. queue peek, etc...
	if (xQueueSend(xUsbhCtrlQueue, &req, 0) != pdTRUE) {
		delete req;
	}
}


