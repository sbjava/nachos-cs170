#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "bitmap.h"

class ProcessManager
{
    public:
        ProcessManager();	// Constructor
        ~ProcessManager();	// Destructor

 	int getPID();		// return an unused PID
    void clearPID(int pid); // clear out PID passed in
	void insertProcess(PCB *pcb, int pid); //insert PCB into pcsb    

    private:
	BitMap processes; 	// a BitMap for process manager methods
	PCB** pcbs;		// an array of PCB* to store the PCBs

}

#endif
