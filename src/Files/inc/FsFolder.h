#ifndef FSFOLDER_H_
#define FSFOLDER_H_


#include <vector>
#include <memory>
#include "FileInfos.h"
#include "ff.h"

typedef struct FILE_DATA_t {
	uint8_t * data;
	uint32_t size;
} FILE_DATA_t;

class FsFolder {
public:
	FsFolder();
	FsFolder(const FsFolder &f2);
	virtual ~FsFolder();
	void setDirPrevious();
	void enterDirectory(TCHAR * dir);
	const std::shared_ptr<std::vector<std::shared_ptr<FileInfos>>> getFiles();
	const std::shared_ptr<std::vector<std::shared_ptr<TCHAR>>> getDirs();
	const TCHAR * getPath();
	const char * getCurrentFileExt();
	void setActiveFile(std::shared_ptr<FileInfos>);
	void setActiveFile(std::shared_ptr<TCHAR> filename);
	void advanceFile();
	FILE_DATA_t getFileData();
	char * getFilename();
private:
	void refreshVectors();
	TCHAR * path;
	uint16_t fIdx;
	std::shared_ptr<std::vector<std::shared_ptr<FileInfos>>> files;
	std::shared_ptr<std::vector<std::shared_ptr<TCHAR>>> subdirs;
};

#endif /* FSFOLDER_H_ */
