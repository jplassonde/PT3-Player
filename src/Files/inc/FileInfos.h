#pragma once
#include "ff.h"
#include <memory>

class FileInfos {
public:
	FileInfos(TCHAR * fn, FSIZE_t size);
	virtual ~FileInfos();
	std::shared_ptr<TCHAR> getName() const;
	FSIZE_t getSize() const;
	void setIndex(uint16_t idx);
	uint16_t getIndex() const;
private:
	std::shared_ptr<TCHAR> filename;
	const FSIZE_t size;
	uint16_t _idx;
};
#include "FileInfos.h"
