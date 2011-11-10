// pcb.cc
//  Implements the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#include "pcb.h"


// PCB::PCB
// Initialize a PCB
PCB::PCB(){
	pid = -1;
	parentPid = -1;
	thread = 0;
	status = -1;
	
	// Make sure to initialize the array of UserOpenFile's
	for(int i = 0; i < MAX_FILES; i++)
		fileArray[i] = 0; 
}

// PCB::~PCB
// Deallocate PCB
PCB::~PCB(){	
	
}

UserOpenFile*
PCB::getFile(int id){
	for(int i=0; i < MAX_FILES; i++)
		if(fileArray[i] != 0 && fileArray[i]->indexPosition == id)
			return fileArray[i];
	
	return 0;
}

int
PCB::Add(UserOpenFile *file){
	int index = -1;
	for(int i = 0; i<MAX_FILES; i++){
		if(fileArray[i] == 0){
			fileArray[i] = file;
			index = i;
		}
	}
	return index;
}

bool
PCB::Remove(int fileId){
	for(int i=0; i < MAX_FILES; i++){
		if(fileArray[i] != 0 && fileArray[i]->indexPosition == fileId){
			delete fileArray[i]->fileName;
		    delete fileArray[i]->offset;
		    delete fileArray[i]->indexPosition;
			fileArray[i]=0;
			return true;
		}			
	}
	return false;
}



