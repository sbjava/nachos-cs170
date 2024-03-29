// pcb.h
//  Defines the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#ifndef PCB_H_
#define PCB_H_

#include "useropenfile.h"
#include "syscall.h"
#include "bitmap.h"

#define MAX_FILES 30

#define PCB_GOOD 0;
#define PCB_BAD 1;
#define PCB_RUNNING 2;
#define PCB_BLOCKED 3;

class Thread;

class PCB {
public:
	int pid;
	int parentPid;
	Thread *thread;
	int status;
	
	
	UserOpenFile *fileArray[MAX_FILES]; 
	//@@@ UserOpenFile* openFiles[MAX_FILES];

	//UserOpenFile* getFile(int id);
	
	PCB();
	PCB(SpaceId currPid, SpaceId currParentPid, Thread* currThread, int currStatus);
	~PCB();
	
	int Add(UserOpenFile *file);
	bool Remove(UserOpenFile *file);
	UserOpenFile* getFile(int index);
	
private:
	BitMap* files;	
};

#endif
