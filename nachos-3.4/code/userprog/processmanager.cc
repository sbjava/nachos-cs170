// processmanager.cc
//
//	ProcessManager class similiar memory manager function, return an unused process
//	id, clear a process id respectively by using a bitmap, store PCBs in an array of PCB*.

//// Might not need to include here
#include "system.h"
/////


#include "processmanager.h"

//----------------------------------------------------------------------
// ProcessManager::ProcessManager
//      Initialize a process manager with a bitmap of size "MAX_PROCESS" 
//      bits and create the PCBs array
//----------------------------------------------------------------------
ProcessManager::ProcessManager():processes(MAX_PROCESSES)
{
    pcbs = new PCB*[MAX_PROCESSES];
}

void ProcessManager::insertProcess(PCB *pcb, int pid){
	pcbs[pid] = pcb;
}

int ProcessManager::getPID()
{
    // Find()- first clear bit in bitmap
    return processes.Find();
}

void ProcessManager::clearPID(int pid)
{
    processes.Clear(pid);
}

ProcessManager::~ProcessManager()
{
	delete pcbs;
}