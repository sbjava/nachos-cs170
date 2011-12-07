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

#define FILE_NAME_SIZE 128

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
extern void PageFaultHandler(int);
void myFork(int);
void myYield();
SpaceId myExec(char *);
int myJoin(int);
void myExit(int);
void myCreate(char *);
OpenFileId myOpen(char *);
int myRead(int, int, OpenFileId);
void myWrite(int, int, OpenFileId);
void myClose(OpenFileId);

char* CloneString(char *old){
	char * newString = new char[FILE_NAME_SIZE];
	for (int i = 0; i < FILE_NAME_SIZE; i++) {
		newString[i] = old[i];
		if (old[i] == NULL)
			break;
	}
	return newString;
}


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
	char* fileName = new char[128];
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
               //machine->WriteRegister(2, );
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
			case SC_Join:
			{
				int arg = machine->ReadRegister(4);
				pid = myJoin(arg);
				machine->WriteRegister(2, pid);
				break;
			}	
			case SC_Exit:
			{
				int arg = machine->ReadRegister(4);
				myExit(arg);
				break;
			}
			case SC_Create:
			{
				int position = 0;
				int arg = machine->ReadRegister(4);
				int value;
				while(value != NULL){
					machine->ReadMem(arg, 1, &value);
					fileName[position] = (char) value;
					position++;
					arg++;
				}
				
				myCreate(fileName);
				break;
			}
			case SC_Open:
			{
				int position = 0;
				int arg = machine->ReadRegister(4);
				int value;
				while(value != NULL){
					machine->ReadMem(arg, 1, &value);
					fileName[position] = (char) value;
					position++;
					arg++;
				}
				//printf("before open: %s\n", fileName);
				OpenFileId index = myOpen(fileName);
				if(index == -1)
					printf("Unable to open file\n");
				machine->WriteRegister(2, index);				
				break;
			}
			case SC_Read:
			{
				int arg1 = machine->ReadRegister(4);
				int arg2 = machine->ReadRegister(5);
				int arg3 = machine->ReadRegister(6);
				int tmp = myRead(arg1, arg2, arg3);
				machine->WriteRegister(2, tmp);
				break;
			}
			case SC_Write:
			{
				int arg1 = machine->ReadRegister(4);
				int arg2 = machine->ReadRegister(5);
				int arg3 = machine->ReadRegister(6);
				myWrite(arg1, arg2, arg3);			
				break;
			}
			case SC_Close:
			{
				int arg1 = machine->ReadRegister(4);
				myClose(arg1);
				break;
			}
		}
		currentThread->space->CleanupSysCall();	
		incrRegs();		
    }else if(which == PageFaultException)
    {
		//printf("**PageFaultException in exception.cc\n");
		PageFaultHandler(machine->ReadRegister(BadVAddrReg));
    }
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}

/////////////////////////////////
// PageFaultExceptionHandler
////////////////////////////////
extern void PageFaultHandler(int vaddr) 
{
    int page = vaddr / PageSize;
    DEBUG('q',"Page fault at page %d\n",page);
    #ifdef VM
    vm->Swap(page, currentThread->space->pcb->pid);
    #endif

}


/////////////////////////////////
// Dummy function used by myFork
/////////////////////////////////
void myForkHelper(int funcAddr) {
	//@@@ ADDED
	currentThread->space->InitRegisters();
	
	currentThread->space->RestoreState(); // load page table register	
	machine->WriteRegister(PCReg, funcAddr);
	machine->WriteRegister(NextPCReg, funcAddr + 4);
	
	//@@@ return address added
	//machine->WriteRegister(RetAddrReg, funcAddr+8);
	
	machine->Run(); // jump to the user progam
	ASSERT(FALSE); // machine->Run never returns;	
}

/////////////////////////////////
// Fork system call
/////////////////////////////////
void myFork(int funcAddr){
	printf("System Call: %d invoked Fork\n", currentThread->space->pcb->pid);
	int currPid = currentThread->space->pcb->pid;
	AddrSpace *space = currentThread->space->Duplicate();
	if(space==NULL){
		printf("Process %d Fork: start at address 0x%X with %d pages memory failed\n", currentThread->space->pcb->pid,funcAddr,space->getNumPages());
		return;
	}
	
	PCB *pcb = new PCB();
	Thread* thread = new Thread("new forked thread.");
	
	pcb->thread = thread;
	pcb->pid = procManager->getPID();
	ASSERT(pcb->pid!=-1);
	pcb->parentPid = currentThread->space->pcb->pid; 
	space->pcb = pcb;
	thread->space = space;
	procManager->insertProcess(pcb, pcb->pid);
	
	//@@@ save state needed?
	space->SaveState();
	
	
	printf("Process %d Fork %d: start at address 0x%X with %d pages memory\n",currPid,pcb->pid,funcAddr,space->getNumPages());
	thread->Fork(myForkHelper, funcAddr);
	currentThread->Yield();	
		
}

// Yield system call
void myYield(){
	printf("System Call: %d invoked Yield\n", currentThread->space->pcb->pid);
	currentThread->Yield();
}

/////////////////////////////////
// Helper func to create new process in register.
/////////////////////////////////
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

/////////////////////////////////
// Exec system call
/////////////////////////////////
SpaceId myExec(char *file){
	printf("System Call: %d invoked Exec\n", currentThread->space->pcb->pid);
	printf("Exec Program: %d loading %s\n", currentThread->space->pcb->pid, file);
	
	int spaceID;
	OpenFile *executable = fileSystem->Open(file);
	
	if(executable == NULL){
		printf("Exec Program: %d loading %s failed\n", currentThread->space->pcb->pid, file);
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

/////////////////////////////////
// Join system call
/////////////////////////////////
int myJoin(int arg){
	printf("System Call: %d invoked Join\n", currentThread->space->pcb->pid);
	
	currentThread->space->pcb->status = BLOCKED;
	
	if(procManager->getStatus(arg) < 0)
		return procManager->getStatus(arg);
		
	procManager->join(arg);
	currentThread->space->pcb->status = RUNNING;
	return procManager->getStatus(arg);	
}

/////////////////////////////////
// Exit system call
/////////////////////////////////
void myExit(int status){
	printf("System Call: %d invoked Exit\n", currentThread->space->pcb->pid);
	printf("Process %d exists with %d\n", currentThread->space->pcb->pid,status);
	int pid = currentThread->space->pcb->pid;
	procManager->Broadcast(pid);
	delete currentThread->space;
	currentThread->space = NULL;
	procManager->clearPID(pid);
	currentThread->Finish();
}

/////////////////////////////////
// Create system call
/////////////////////////////////
void myCreate(char *fileName){
	printf("System Call: %d invoked Create\n", currentThread->space->pcb->pid);
	bool fileCreationWorked = fileSystem->Create(fileName, 0);
	ASSERT(fileCreationWorked);
}

/////////////////////////////////
// Open system call
/////////////////////////////////
OpenFileId myOpen(char *name){
	printf("System Call: %d invoked Open\n", currentThread->space->pcb->pid);
	
	int index = 0;	
	SysOpenFile *sFile = openFileManager->Get(name, index);
	//printf("open sys call\n");
	
	//@@@
	/*for(int i = 2; i < MAX_FILES; i++){
		//printf("opening at %i\n", i);
		//checking to see if file already open
		if(openFilesArray[i] != NULL && strcmp(openFilesArray[i]->fileName, name)==0){
			//printf("opened2 at %i\n", i);
			sFile = openFilesArray[i];
			index = i;
		}		
	}*/
	
	if (sFile == NULL) {
		//printf("open3 at\n");
		OpenFile *oFile = fileSystem->Open(name);
		sFile = new SysOpenFile(name, oFile, -1);
		index = openFileManager->Add(sFile);
		//printf("open4 at\n");
		/*		
		if (oFile == NULL) {
			// Here if file is unable to be opened
			//printf("ofile is %p\n", oFile);
			return -1;
		}
		//printf("open5 at\n");
		*/
		
		/*SysOpenFile *sysFile = new SysOpenFile();
		sysFile->file = oFile;
		sysFile->numUsersAccess = 1;
		sysFile->fileName = CloneString(name);
		//printf("open6 at\n");
		//char* tmp[strlen(name)];
		
		//strcpy(tmp, name);
		//printf("open7 at %s\n", sysFile->fileName);
		for(int i = 2; i < MAX_FILES; i++){
			//printf("openingAGAIN at %i\n", i);
			if(openFilesArray[i] == NULL){
				//Opened file placed at [i]
				//printf("openFilesArray[i]: %p\n", openFilesArray[i]);
				openFilesArray[i] = sysFile;
				index = i;
				break;
			}
		}*/		
	}
	else{
		sFile->numUsersAccess++;
	}
	//printf("open8 at\n");
	UserOpenFile *uFile = new UserOpenFile();
	uFile->indexPosition = index;
	uFile->offset = 0;
	uFile->fileName = CloneString(name);
	//printf("open9 at: %s\n", uFile->fileName);	
	OpenFileId oFileId = currentThread->space->pcb->Add(uFile);
	//printf("open10 at: %i\n", oFileId);	
	return oFileId;	
}

/////////////////////////////////
// Read system call
/////////////////////////////////
int myRead(int bufferAddress, int size, OpenFileId id){
	printf("System Call: %d invoked Read\n", currentThread->space->pcb->pid);
	char *buffer = new char[size + 1];
	int sizeCopy = size;
	
	if (id == ConsoleInput) {
		int count = 0;

		while (count < size) {
			buffer[count]=getchar();
			count++;
		}
		
		
	} 
	else {		
		UserOpenFile* userFile =  currentThread->space->pcb->getFile(id);
		if(userFile == NULL){
			return -1;
			// error
		}
		SysOpenFile *sFile;	
			
		if(openFileManager->Get(userFile->indexPosition)->fileID == id)	
			sFile = openFileManager->Get(userFile->indexPosition);	
		else
			return -1;
	
		if(sFile == NULL) {
			return -1;
			//error
		}
		//new fileManager = openFilesArray
		/*if(openFilesArray[userFile->indexPosition] != NULL)
			sFile = openFilesArray[userFile->indexPosition];*/
		
		//printf("reading sFile->file: %d\n", sFile->file);
		//printf("reading offset: %d\n",userFile->offset);
		//printf("reading buffer: %p\n",buffer);
		//printf("reading size: %d\n",size);
		sFile->lock->Acquire();					
		sizeCopy = sFile->file->ReadAt(buffer,size,userFile->offset);
		sFile->lock->Release();					
		//printf("reading is now here\n");
		userFile->offset+=sizeCopy;
		//@@@ need buffer[sizeCopy] = '/0' ????
	}
	//---------Need a read write func----------
	//int ReadWrite(int virAddr, OpenFile *file, int size, int fileAddr, int type)	
	//ReadWrite(bufferAddress, userFilesArray[id]->file, sizeCopy, )
	ReadWrite(bufferAddress,buffer,sizeCopy,USER_READ);
	//-----------------------------------------
	delete[] buffer;
	return sizeCopy;	
}

/////////////////////////////////
// Write system call
/////////////////////////////////
void myWrite(int bufferAddress, int size, OpenFileId id){
	printf("System Call: %d invoked Write\n", currentThread->space->pcb->pid);
	char* buffer = new char[size+1];
	
	ReadWrite(bufferAddress,buffer,size,USER_WRITE);

	
	
	if (id == ConsoleOutput) {
		
		buffer[size]=0;
		printf("%s",buffer);
		
	} 
	else {
		buffer = new char[size];
		int writeSize = ReadWrite(bufferAddress,buffer,size,USER_WRITE);
		UserOpenFile* uFile =  currentThread->space->pcb->getFile(id);
		if(uFile == NULL)
			return;
		
		SysOpenFile *sFile;	
		if(openFileManager->Get(uFile->indexPosition)->fileID == id)	
			sFile = openFileManager->Get(uFile->indexPosition);
		else
			return;
		
		/*if(openFilesArray[uFile->indexPosition] != NULL)
			sFile = openFilesArray[uFile->indexPosition];*/
			
		if(sFile == NULL){
			return;
			//error
		}	
			
		int bytes = sFile->file->WriteAt(buffer,size,uFile->offset);
		uFile->offset+=bytes;
	}
	delete[] buffer;	
}


/////////////////////////////////
// Close system call
/////////////////////////////////
void myClose(OpenFileId id){
	printf("System Call: %d invoked Close\n", currentThread->space->pcb->pid);
	UserOpenFile* userFile =  currentThread->space->pcb->getFile(id);
	if(userFile == NULL){
		return ;
	}
	//int tmpIndex = userFile->indexPosition;

	SysOpenFile *sFile;
	if(openFileManager->Get(userFile->indexPosition)->fileID == id)	
		sFile = openFileManager->Get(userFile->indexPosition);
	else
		return;
		
	if(sFile == NULL){
		return;
	}
	/*if(openFilesArray[userFile->indexPosition] != NULL)
		sFile = openFilesArray[tmpIndex];*/	
	
	//sFile->closeOne();
	currentThread->space->pcb->Remove(userFile);
	//delete openFilesArray[tmpIndex];
	//openFilesArray[tmpIndex] = NULL;
}

