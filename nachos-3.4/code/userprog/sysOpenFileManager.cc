#include "sysOpenFileManager.h"

SysOpenFileManager::SysOpenFileManager() {
	openFiles = new BitMap(SOFILETABLE_SIZE);
	lock = new Lock("SysOpenFileManager Lock");
}

SysOpenFileManager::~SysOpenFileManager() {
	delete lock;
	delete openFiles;
}

int SysOpenFileManager::Add(SysOpenFile* newSysOpenFile) {
	lock->Acquire();
	int index = openFiles->Find();

	if (index < 0) {
		// THROW AN ERROR
		lock->Release();
		return -1;
	}

	openFileTable[index] = new SysOpenFile();
	newSysOpenFile->fileID = index + 2;

	lock->Release();

	return index;
}

SysOpenFile* SysOpenFileManager::Get(char* fileName, int& index) {
	lock->Acquire();
	for (int i=0; i < SOFILETABLE_SIZE; i++) {
		if (!openFiles->Test(i)) 
			break;
		if (strcmp(sysOpenFileTable[i]->fileName, fileName) == 0) 
		{
			index = i;
			lock->Release();
			return openFileTable[i];
		}
	}

	lock->Release();

	return NULL;
}

SysOpenFile* SysOpenFileManager::Get(int index) {
	lock->Acquire();
	if (index >= SOFILETABLE_SIZE) {
		// THROW AN ERROR
		lock->Release();
		return NULL;
	}

	if (openFiles->Test(index)) {
		lock->Release();
		return NULL;
	}

	lock->Release();

	return openFileTable[index];
}

void SysOpenFileManager::Close(int index) {
	lock->Acquire();
	if (index >= SOFILETABLE_SIZE) {
		// THROW AN ERROR
		lock->Release();
		return;
	}

	if (openFiles->Test(index)) {
		openFileTable[index]->userOpens--;
		if (openFileTable[index]->userOpens == 0) 
		{
			delete openFileTable[index]->file;
			delete openFileTable[index];
			openFileTable[index] = NULL;
			openFiles->Clear(index);
		}
	}

	lock->Release();

}