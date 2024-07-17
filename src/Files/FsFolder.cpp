#include <cstring>
#include <algorithm>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "FsFolder.h"

extern SemaphoreHandle_t xFatFsMutex;

bool compareFileInfos(std::shared_ptr<FileInfos> f1, std::shared_ptr<FileInfos> f2);
bool compareDirNames(std::shared_ptr<TCHAR> d1, std::shared_ptr<TCHAR> d2);

constexpr uint8_t ROOT_DIR_LENGTH = 6;


FsFolder::FsFolder() {
	files = std::make_shared<std::vector<std::shared_ptr<FileInfos>>>();
	subdirs = std::make_shared<std::vector<std::shared_ptr<TCHAR>>>();
	path = (TCHAR *)"";
	TCHAR _path[] = "files";
	enterDirectory(_path);
}

FsFolder::FsFolder(const FsFolder &f2) {
	path = (TCHAR *)pvPortMalloc((strlen(f2.path) + 1) * sizeof(TCHAR));
	strcpy(path, f2.path);
	fIdx = f2.fIdx;
	files = f2.files;
	subdirs = f2.subdirs;
}

FsFolder::~FsFolder() {
	vPortFree(path);
}

void FsFolder::setDirPrevious() {
	uint8_t lastSlash = 0;
	int i = ROOT_DIR_LENGTH;
	while(path[i]) {
		if (path[i] == '/') {
			lastSlash = i;
		}
		++i;
	}
	if (lastSlash != 0) {
		path[lastSlash] = 0;
		refreshVectors();
	}
}

void FsFolder::enterDirectory(TCHAR * dir) {
	TCHAR * newPath = (TCHAR*)pvPortMalloc((strlen(path)+strlen(dir)+2)*sizeof(TCHAR));
	strcpy(newPath, path);
	strcat(newPath, "/");
	strcat(newPath, dir);
	vPortFree(path);
	path = newPath;
	refreshVectors();
}

const std::shared_ptr<std::vector<std::shared_ptr<FileInfos>>> FsFolder::getFiles() {
	return files;
}

const std::shared_ptr<std::vector<std::shared_ptr<TCHAR>>> FsFolder::getDirs() {
	return subdirs;
}

void FsFolder::setActiveFile(std::shared_ptr<FileInfos> file) {
	fIdx = file->getIndex();
}

void FsFolder::setActiveFile(std::shared_ptr<TCHAR> filename) {
	for (auto i : *files) {
		if (i->getName().get() == filename.get()) {
			fIdx = i.get()->getIndex();
			return;
		}
	}
}

void FsFolder::advanceFile() {
	++fIdx;
	if (fIdx >= files->size()) {
		fIdx = 0;
	}
}

FILE_DATA_t FsFolder::getFileData() {
	FATFS fs;
	FIL fp;
	UINT numRead;

	FILE_DATA_t fdata;
	fdata.size = files->at(fIdx)->getSize();

	fdata.data = (uint8_t *)pvPortMalloc(fdata.size*sizeof(TCHAR));

	xSemaphoreTake(xFatFsMutex, portMAX_DELAY);

	f_mount(&fs, "", 0);
	f_chdir(path);
	f_open(&fp, files->at(fIdx)->getName().get(), FA_READ);
	f_read(&fp, (void*)fdata.data, fdata.size, &numRead);
	f_close(&fp);
	f_mount(0, "", 0);

	xSemaphoreGive(xFatFsMutex);

	return fdata;
}

char * FsFolder::getFilename() {
	return files->at(fIdx)->getName().get();
}

void FsFolder::refreshVectors() {
	FATFS fs;
	FRESULT res;
	DIR dp;
	FILINFO fno;

	files = std::make_shared<std::vector<std::shared_ptr<FileInfos>>>();
	subdirs = std::make_shared<std::vector<std::shared_ptr<TCHAR>>>();

	xSemaphoreTake(xFatFsMutex, portMAX_DELAY);

	f_mount(&fs, "", 0);
	res = f_opendir(&dp, path);

	while(1) {
		res = f_readdir(&dp, &fno);
		if (res != FR_OK || fno.fname[0] == 0) {
			break;
		}

		if (fno.fattrib & AM_DIR) {
			std::shared_ptr<TCHAR> name((TCHAR*)pvPortMalloc((strlen(fno.fname)+1)*sizeof(TCHAR)));
			strcpy(name.get(), fno.fname);
			subdirs->push_back(name);
		} else {
			std::shared_ptr<FileInfos> fi(new FileInfos(fno.fname, fno.fsize));
			files->push_back(fi);
		}
	}

	f_closedir(&dp);
	f_mount(0, "", 0);

	xSemaphoreGive(xFatFsMutex);

	std::sort(files->begin(), files->end(), compareFileInfos);
	std::sort(subdirs->begin(), subdirs->end(), compareDirNames);

	for (uint16_t i = 0; i < files->size(); i++) {
		files->at(i)->setIndex(i);
	}
}

const TCHAR * FsFolder::getPath() {
	return path;
}

const char * FsFolder::getCurrentFileExt() {
	return strrchr(files->at(fIdx)->getName().get(), '.') + 1;
}


bool compareFileInfos(std::shared_ptr<FileInfos> f1, std::shared_ptr<FileInfos> f2) {
	for (uint8_t i = 0; i <= strlen(f1->getName().get()); i++) {
		if (toupper(f1->getName().get()[i]) != toupper(f2->getName().get()[i])) {
			return toupper(f1->getName().get()[i]) < toupper(f2->getName().get()[i]);
		}
	}
	return true;
}

bool compareDirNames(std::shared_ptr<TCHAR> d1, std::shared_ptr<TCHAR> d2) {
	for (uint8_t i = 0; i <= strlen(d1.get()); i++) {
		if (toupper(d1.get()[i]) != toupper(d2.get()[i])) {
			return toupper(d1.get()[i]) < toupper(d2.get()[i]);
		}
	}
	return true;
}

