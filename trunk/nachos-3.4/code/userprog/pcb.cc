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
	pid = 0;
	parentPid = 0;
	thread = NULL;
}

// PCB::~PCB
// Deallocate PCB
PCB::~PCB(){	
	
}




