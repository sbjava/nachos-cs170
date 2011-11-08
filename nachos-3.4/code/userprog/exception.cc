// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void myFork(int);
void myYield();
SpaceId myExec(char *);

void incrRegs(){
	int pc = machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg, pc);
	pc = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, pc);
	machine->WriteRegister(NextPCReg, pc + 4);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	char fileName[128];
	int pid;
	
    if(which == SyscallException)
    {
        switch(type)
        {
	    	case SC_Halt:
           	{
	       		DEBUG('a', "Shutdown, initiated by user program.\n");
	       		interrupt->Halt();
               	break;
            }
            case SC_Fork:
            {
               myFork(machine->ReadRegister(4));
               //machine->WriteRegister(2, result);
               break;
            }
			case SC_Yield:
			{
				myYield();
				break;
			}
			case SC_Exec:
			{
				//printf("in myexec\n");
				int position = 0;
			    int arg = machine->ReadRegister(4);
			    int value;
			    while (value != NULL) {
			    	machine->ReadMem(arg, 1, &value);
			        fileName[position] = (char) value;
			        position++;
			        arg++;
			    }
				
				pid = myExec(fileName);
				//printf("pid returned from myExec: %d\n", pid);
				machine->WriteRegister(2, pid);
				break;
			}
		}	
		incrRegs();		
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}

// Dummy function used by myFork
void myForkHelper(int funcAddr) {
	/*int* stateData = (int*)funcAddr;
	for(int i=0; i<NumTotalRegs; i++)
		machine->WriteRegister(i, stateData[i]);
	delete[] stateData;
	
	currentThread->space->RestoreState();
	machine->Run();
	ASSERT(FALSE);*/
	// ^ Old
	currentThread->space->RestoreState(); // load page table register
	machine->WriteRegister(PCReg, funcAddr);
	machine->WriteRegister(NextPCReg, funcAddr + 4);
	machine->Run(); // jump to the user progam
	ASSERT(FALSE); // machine->Run never returns;	
}

// Fork system call
void myFork(int funcAddr){
	/*
	// Check funcAddr
	if(funcAddr < 0) {
		printf("The address of this function is invalid.");
		return;
	}
	AddrSpace *addrSpace = currentThread->space;
	Thread *thread = new Thread("forked thread");	
	thread->space = addrSpace;
	
	// save PC, return, and other machine registers before fork and yield
	addrSpace->SaveState();
	int* currStateData = new int[NumTotalRegs];
	for(int i=0; i<NumTotalRegs; i++)
		currStateData[i] = machine->ReadRegister(i);
		
	currStateData[PCReg] = funcAddr;
	currStateData[NextPCReg] = funcAddr+4;		
		
	thread->Fork(myForkHelper, (int)currStateData);		
	currentThread->Yield();	*/
	//^Old
	AddrSpace *space = currentThread->space->Duplicate();
	if(space==NULL)
		return;
	
	PCB *pcb = new PCB();
	Thread* thread = new Thread("new forked thread.");
	
	pcb->thread = thread;
	pcb->pid = procManager->getPID();
	ASSERT(pcb->pid!=-1);
	pcb->parentPid = currentThread->space->pcb->pid; 
	space->pcb = pcb;
	thread->space = space;
	procManager->insertProcess(pcb, pcb->pid);
	space->SaveState();
	thread->Fork(myForkHelper, funcAddr);
	currentThread->Yield();	
		
}

// Yield system call
void myYield(){
	currentThread->Yield();
}

// Helper func to create new process in register.
void newProc(int arg){
	/*printf("***exec stuf\n");
	printf("currentThread: %p\n", currentThread);
	printf("currentThread->space: %p\n", currentThread->space);
	printf("currentThread->space->pcb: %p\n", currentThread->space->pcb);
	printf("currentThread->space->pcb->pid: %i\n", currentThread->space->pcb->pid);*/
	currentThread->space->InitRegisters(); 
	currentThread->space->SaveState();
	currentThread->space->RestoreState(); 
	//printf("before run\n");
	machine->Run();	
}

// Exec system call
SpaceId myExec(char *file){
	//printf("passed to exec: %s\n", file);
	int spaceID;
	OpenFile *executable = fileSystem->Open(file);
	
	if(executable == NULL){
		printf("Unable to open file %s\n", file);
		return -1;
	}
	AddrSpace *space;
	space = new AddrSpace(executable);	
	
	PCB* pcb = new PCB();
	pcb->pid = procManager->getPID();		
	Thread *t = new Thread("Forked process");
	pcb->pid = procManager->getPID();
	spaceID = pcb->pid;	
	ASSERT(pcb->pid!=-1);
	
	/*
	printf("currentThread: %p\n", currentThread);
	printf("currentThread->space: %p\n", currentThread->space);
	printf("currentThread->space->pcb: %p\n", currentThread->space->pcb);
	printf("currentThread->space->pcb->pid: %i\n", currentThread->space->pcb->pid);
	*/
	pcb->parentPid = currentThread->space->pcb->pid;	
	pcb->thread = t;
	space->pcb = pcb;	
	t->space = space;	
	procManager->insertProcess(pcb, pcb->pid);
	delete executable;
	t->Fork(newProc, NULL);	
	currentThread->Yield();
	return spaceID;	
}


