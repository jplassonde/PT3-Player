// Only handle control out requests for now, and will mostly stay this way
// unless/until I figure out how (if even possible) to send Ctrl & Iso in the same frame.
// JK I FIXXED IT, more code to write now :)

#include "project.h"
#include "ControlReq.h"
#include <cstring>
#include "usbh_ioreq.h"
#include "usbh_g935.h"

extern DebugInfo_t ctrlReqInfo;

ControlReq::ControlReq(Init_t * init) :
		ctrlState(Setup), bmReqType(init->bmRT), bRequest(init->bReq),
		wValue(init->wVal), wIndex(init->wIdx), wLength(init->wLen) {


	if (wLength > 0) {
		dataOut = (uint8_t *)pvPortMalloc(wLength * sizeof(uint8_t));
		memcpy(dataOut, init->dat, wLength);
	} else {
		dataOut = nullptr;
	}
}

ControlReq::~ControlReq(){
	if (wLength > 0) {
		vPortFree(dataOut);
	}
}

bool ControlReq::processRequest(USBH_HandleTypeDef *phost) {

	uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;
	ctrlReqInfo.FRNUM_Start = USBx_HOST->HFNUM & 0xFFFF;
	ctrlReqInfo.FTREM_Start = (USBx_HOST->HFNUM >> 16) & 0xFFFF;
	ctrlReqInfo.HFIR = USBx_HOST->HFIR;
	ctrlReqInfo.HNPTXSTS_Start = USB_OTG_HS->HNPTXSTS;
	ctrlReqInfo.HPTXSTS_Start = USBx_HOST->HPTXSTS;

	switch (ctrlState) {
		case Setup:
			sendSetup(phost);
			return false;
			break;

		case SetupSent:
			checkSetupStatus(phost); // Check status and get/receive data if ready
			break;

		case SendData:
			sendData(phost);
			break;

		case RxDataStatus: {
			break;
		}
		case TxDataStatus: {
			bool status = checkDataOutStatus(phost);
			return status;
			break;
		}
		default:
			while(1);
			break;
	}
	ctrlReqInfo.HNPTXSTS_End = USB_OTG_HS->HNPTXSTS;
	ctrlReqInfo.HPTXSTS_End = USBx_HOST->HPTXSTS;
	ctrlReqInfo.FRNUM_End = USBx_HOST->HFNUM & 0xFFFF;
	ctrlReqInfo.FTREM_End = (USBx_HOST->HFNUM >> 16) & 0xFFFF;
	return false;
}

void ControlReq::sendSetup(USBH_HandleTypeDef *phost) {
	uint8_t setupPkt[8];
	setupPkt[0] = bmReqType;
	setupPkt[1] = bRequest;
	*(uint16_t*)&setupPkt[2] = wValue;
	*(uint16_t*)&setupPkt[4] = wIndex;
	*(uint16_t*)&setupPkt[6] = wLength;

	HAL_HCD_HC_SubmitRequest((HCD_HandleTypeDef*)phost->pData,
							 phost->Control.pipe_out,
							 0, // out
							 USBH_EP_CONTROL,
							 USBH_PID_SETUP,
							 setupPkt,
							 USBH_SETUP_PKT_SIZE,
							 0);
	ctrlState = SetupSent;
}

void ControlReq::checkSetupStatus(USBH_HandleTypeDef *phost) {
	USBH_URBStateTypeDef URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_out);

	if (URB_Status == USBH_URB_DONE) {
		uint8_t direction = bmReqType & USB_REQ_DIR_MASK;

		if (wLength != 0) {
			if (direction == USB_D2H) {
				ctrlState = RxDataStatus;
			} else {
				ctrlState = SendData;
			}
		}
	} else { // URB not done
		handleUrbState(URB_Status);
	}
}

void ControlReq::getData(USBH_HandleTypeDef *phost) {

}

void ControlReq::sendData(USBH_HandleTypeDef *phost) {
	HAL_HCD_HC_SubmitRequest((HCD_HandleTypeDef*)phost->pData,
							 phost->Control.pipe_out,
							 0,
							 USBH_EP_CONTROL,
							 USBH_PID_DATA,
							 dataOut,
							 wLength,
							 0);
	ctrlState = TxDataStatus;
}

bool ControlReq::checkDataOutStatus(USBH_HandleTypeDef *phost) {
	USBH_URBStateTypeDef URB_Status = USBH_LL_GetURBState(phost, phost->Control.pipe_out);

	if  ( URB_Status == USBH_URB_DONE) {
		return true;
	} else if ( URB_Status == USBH_URB_NOTREADY) {
		ctrlState = Setup;
	}
	return false;
}

void ControlReq::handleUrbState(USBH_URBStateTypeDef URB_Status) {
	// Not handled. It's a lie.
}
