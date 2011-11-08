#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H


#include "bitmap.h"
#define MAX_PROCESSES PageSize //<< needs to be changed to PageSize

class PCB;
class ProcessManager
{
public:
    ProcessManager();	// Constructor
    ~ProcessManager();	// Destructor

	int getPID();		// return an unused PID
	void clearPID(int pid); // clear out PID passed in
	void insertProcess(PCB *pcb, int pid); //insert PCB into pcsb    
 	void join(int pid);	// keeps track of who is waiting on a condition variable for each PCB
	int getStatus(int pid); // Get pid's process status

private:
	BitMap processes; 	// a BitMap for process manager methods
	PCB** pcbs;		// an array of PCB* to store the PCBs
	Condition **conditions;	// for join and exit
	Lock **locks;		// for join and exit
	int pcbStatus[MAX_PROCESESS];		// array of PCB statuses
	int joinProcessNum[MAX_PROCESESS];	// array of join process numbers
};

#endif
