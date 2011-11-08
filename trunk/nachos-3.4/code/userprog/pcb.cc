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
}

// PCB::~PCB
// Deallocate PCB
PCB::~PCB(){	
	
}




