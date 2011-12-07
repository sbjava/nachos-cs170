// pcb.cc
//  Implements the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#include "pcb.h"
#include "system.h"



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

PCB::PCB(SpaceId currPid, SpaceId currParentPid, Thread* currThread, int currStatus){
	pid = currPid;
	parentPid = currParentPid;
	thread = currThread;
	status = currStatus;

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
	/*for(int i = 2; i<MAX_FILES; i++){
		if(fileArray[i] == 0){
			fileArray[i] = file;
			return i;
		}
	}
	return -1;*/
		int index = files->Find();
		
		if(index > -1){
			fileArrays[index] = file;
		}
		
		return index;
}

bool
PCB::Remove(UserOpenFile* file){
	for(int i=0; i < MAX_FILES; i++){
		if(files->Test(i) && fileArray[i] == file){
		    //fileArray[i]->offset = 0;
		    //fileArray[i]->indexPosition = 0;
			//@@@
			openFileManager->Close(file->indexPosition);
			delete fileArray[i];			
			files->Clear(i);
			return true;
		}			
	}
	return false;
}



