// sysopenfile.cc
//     Defines SysOpenFile class that contains a pointer to the file 
//     system's OpenFile object and (char *)fileName for that file and 
//     the number of user processes currently accessing it. Declare an 
//     array of SysOpenFile objects for use by all system calls implemented.

#include "sysopenfile.h"

//----------------------------------------------------------------------
// SysOpenFile::closeOne
//      close a SysOpenFile and accessing processes
//----------------------------------------------------------------------
/*
void SysOpenFile::closeOne() 
{
    if(numUsersAccess <= 0)
		return;
    numUsersAccess--;
    if (numUsersAccess == 0) 
    {
        delete fileName;
        delete file;
    }
}
//----------------------------------------------------------------------
// SysOpenFile::close
//      close all SysOpenFile and accessing processes
//----------------------------------------------------------------------
void SysOpenFile::close() 
{
    if (numUsersAccess <= 0)
	return;
    numUsersAccess = 0;
    delete fileName;
    delete file;
}
*/

SysOpenFile::SysOpenFile()
{
	file = 0; 	
    fileName = 0;   
    numUsersAccess = 0;
	fileID = 0;
	//@@@
	lock = new Lock("lock");
	//usedFiles = 0;	
}

SysOpenFile::SysOpenFile(char * name, OpenFile* f, int id)
{
	file = f; 	
    fileName = name;   
    numUsersAccess = 1;
	fileID = id;
	//@@@
	lock = new Lock("lock");
	//usedFiles = 0;	
}



SysOpenFile::~SysOpenFile()
{
	delete lock;
}

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
		if (strcmp(openFileTable[i]->fileName, fileName) == 0) 
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
		openFileTable[index]->numUsersAccess--;
		if (openFileTable[index]->numUsersAccess == 0) 
		{
			delete openFileTable[index]->file;
			delete openFileTable[index];
			openFileTable[index] = NULL;
			openFiles->Clear(index);
		}
	}

	lock->Release();

}
