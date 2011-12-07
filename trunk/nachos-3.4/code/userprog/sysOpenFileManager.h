#ifdef SYSOPENFILEMANAGER_H
#define SYSOPENFILEMANAGER_H

#include "bitmap.h"
#include "sysopenfile.h"
#include "synch.h"
//class Lock;

class SysOpenFileManager {
	public:
		SysOpenFileManager();
		~SysOpenFileManager();

		int add(SysOpenFile* newSysOpenFile);
		SysOpenFile* get(char* fileName, int& index);
		SysOpenFile* get(int index);

		Lock* lock;

		void Close(int index);

	private:
		SysOpenFile* openFileTable[SOFIKKLETABLE_SIZE];
		BitMap* openFiles;
};

#endif