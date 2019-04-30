#include "FreeRTOS.h"
#include "task.h"
#include "ff.h"
#include <cstring>
#include "FileInfos.h"

FileInfos::FileInfos(TCHAR * fn, FSIZE_t size) : size(size){
	filename = std::shared_ptr<TCHAR>((TCHAR *)pvPortMalloc((strlen(fn)+1) * sizeof(TCHAR)));
	strcpy(filename.get(), fn);
	_idx = 0;
}

FileInfos::~FileInfos() {
}

std::shared_ptr<TCHAR> FileInfos::getName() const {
	return filename;
}

FSIZE_t FileInfos::getSize() const {
	return size;
}

void FileInfos::setIndex(uint16_t idx) {
	_idx = idx;
}

uint16_t FileInfos::getIndex() const {
	return _idx;
}
