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
    conditions = new Condition*[MAX_PROCESSES];
    locks = new Lock*[MAX_PROCESSES];
}


ProcessManager::~ProcessManager()
{
    delete pcbs;
    delete conditions;
    delete locks;
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

void
join(int pid)
{
    Lock *l = locks[pid];
    if(l == NULL)
    { 
	l = new Lock("");
	locks[pid] = l;
    }
    Condition* c = conditions[pid];
    if(c == NULL)
    {
	c = new Condition("");
	conditions[pid] = c;
    }
    l->Acquire();
    this->joinProcessNum[pid]++;
    c->Wait(l);
    this->joinProcessNum[pid]--;
    if(this->joinProcessNum[pid]==0)
	bitMap.Clear(pid);
    l->Release();

}


int ProcessManager::GetStatus(int pid)
{
    if(bitMap.Test(pid) == 0)
    	return -1;
    return pcbStatus[pid];
}

