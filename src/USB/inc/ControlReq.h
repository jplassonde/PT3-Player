#pragma once

#include <cstdint>
#include "usbh_def.h"

class ControlReq {
public:
	typedef struct Init_t {
		uint8_t bmRT;
		uint8_t bReq;
		uint16_t wVal;
		uint16_t wIdx;
		uint16_t wLen;
		uint8_t * dat;
	} Init_t;

	ControlReq(Init_t * init);
	virtual ~ControlReq();
	bool processRequest(USBH_HandleTypeDef *phost);
private:
	void sendSetup(USBH_HandleTypeDef *phost);
	void checkSetupStatus(USBH_HandleTypeDef *phost);
	void sendData(USBH_HandleTypeDef *phost);
	bool checkDataOutStatus(USBH_HandleTypeDef *phost);
	void getData(USBH_HandleTypeDef *phost);
	void handleUrbState(USBH_URBStateTypeDef URB_Status);

	enum ControlState {Setup, SetupSent, SendData, TxDataStatus, RxDataStatus};
	enum ControlState ctrlState;
	uint8_t bmReqType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint8_t * dataOut;
};

