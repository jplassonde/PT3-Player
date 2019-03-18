#include "ff.h"
#include "diskio.h"
#include "sdCard.h"

extern SDCard * sdCard;

#ifdef __cplusplus
extern "C" {
#endif

DSTATUS disk_status(BYTE pdrv) {
	if (pdrv == 0) {
		return sdCard->getStatus();
	} else {
		return STA_NODISK;
	}
}

DSTATUS disk_initialize(BYTE pdrv) { // SD Card should already be initialized, just return status.
	return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {

	if (pdrv != 0) {
		return RES_PARERR;
	}

	return sdCard->sdRead((uint8_t *)buff, (uint32_t)sector, (uint32_t)count);
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
	// Nothing to do here with current settings (fixed sector size, readonly...)
	return RES_OK;
}

#ifdef __cplusplus
}
#endif
