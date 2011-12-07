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
	processes = new BitMap(MAX_PROCESSES);

	//@@@ lock
	pmLock = new Lock("pm lock")
}


ProcessManager::~ProcessManager()
{
    delete pcbs;
    delete conditions;
    delete locks;
	delete processes;
	delete pmLock;
}

void ProcessManager::insertProcess(PCB *pcb, int pid){
	//@@@
	pmLock->Acquire();
	if(pcb == null)
		return;
	
	pcbs[pid] = pcb;
	conditions[pid] = new Condition("condition");
	locks[pid] = new Lock("lock");
	joinProcessNum[pid] = 0;
 	
	//@@@
	pmLock->Release();
}

int ProcessManager::getPID()
{
    // Find()- first clear bit in bitmap
    return processes.Find();
}

void ProcessManager::clearPID(int pid)
{
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(!processes->Test(pid)){
		pmLock->Release();
		return;
	}
	locks[pid]->Acquire();

	pcbs[pid] = NULL;
	delete conditions[pid];
	conditions[pid] = NULL;
	
	locks[pid]->Release();
	
	delete locks[pid];
	locks[pid] = NULL;
	
    processes.Clear(pid);

	//@@@
	pmLock->Release();
}

void
ProcessManager::join(int pid)
{
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(!processes->Test(pid)){
		pmLock->Release();
		return;
	}
	locks[pid]->Acquire();
	
	joinProcessNum[pid] += 1;
	
	pmLock->Release();
		
	while(1) {
		pmLock->Acquire();
		if(pcbs[pid]->getStatus() > 1){
			locks[pid]->Release();
			pmLock->Release();
			return;
		}
		pmLock->Release();
		conditions[pid]->Wait(locks[pid]);
	}
	
	/*
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
	processes.Clear(pid);
    l->Release();
	*/
}

int ProcessManager::getJoins(int pid) {
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(processes->Test(pid)){
		pmLock->Release();
		return joinProcessNum[pid];
	}
	
	pmLock->Release();
	return -1;
}

void ProcessManager::decrementJoins(int pid){
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(!processes->Test(pid)){
		pmLock->Release();
		return;
	}
	
	if(processes->Test(pid)){
		if(joinProcessNum[pid] > 0 ){
			joinProcessNum[pid] -= 1;
		}
	}
	
	pmLock->Release();
	
}


int ProcessManager::getStatus(int pid)
{
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(!processes->Test(pid)){
		pmLock->Release();
		return;
	}

	pmLock->Release();
	/*
    if(processes.Test(pid) == 0)
    	return -1;
    return pcbStatus[pid];*/

	return pcbs[pid]->getStatus();

}

void ProcessManager::Broadcast(int pid)
{
	//@@@
	pmLock->Acquire();
	
	if(pid > MAX_PROCESSES){
		pmLock->Release();
		return;
	}
	
	if(!processes->Test(pid)){
		pmLock->Release();
		return;
	}
	
	locks[pid]->Acquire();
	conditions[pid]->Broadcast(locks[pid]);
	
	locks[pid]->Release();
	
	pmLock->Release();
	return;
		
	/*
    Lock * lock = locks[pid];
    Condition* condition = conditions[pid];
    pcbStatus[pid] = pcbs[pid]->status;
    if(condition != NULL)
    {
        lock->Acquire();
        condition->Broadcast(lock);
        lock->Release();
    }*/
}

