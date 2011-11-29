// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;
    int read;
 
    pcb = new PCB(procManager->getPID(), -1, NULL, -1); // not even sure if pcb has this constr
    procManager->AddProcess(pcb);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    // we need to increase the size
    // to leave room for the stack
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	
    
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    #ifndef VM
   	ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
	if(numPages > memManager->getAvailable())
        {
	    printf("Not enough memory\n");
	    procManager->clearPID(pcb->getPID()); // clearPID() ??
            numPages = -1;
	    return;
	} // ^^^ is this a good error catcher ^^^
    #endif

   DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) 
    {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	
        #ifdef VM // initialize process with no pages in memory
        pageTable[i].physicalPage = -1;
        pageTable[i].valid = FALSE;
        #else
        pageTable[i].physicalPage = memManager->getPage();  // memManager defined in system.cc
	pageTable[i].valid = TRUE;
        #endif

	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
//    bzero(machine->mainMemory, size); // needed or not??

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",noffH.code.virtualAddr, noffH.code.size);//
        //executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),noffH.code.size, noffH.code.inFileAddr);
		read = ReadFile(noffH.code.virtualAddr,executable,noffH.code.size,noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",noffH.initData.virtualAddr, noffH.initData.size);
        //executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),noffH.initData.size, noffH.initData.inFileAddr);
		read = ReadFile(noffH.initData.virtualAddr,executable,noffH.initData.size,noffH.initData.inFileAddr);
    }

    #ifdef VM
    char* buffer = new char[PageSize + 1];
    memset(buffer, '\0', PageSize);
    
    
    int page = ceil((float)(noffH.code.size + noffH.initData.size)/PageSize);
    DEBUG('q', "######################Adding %d pages\n",(numPages - page));
    
    for (int v = page; v < numPages; v++)
        vm->AddPage(&pageTable[v], pcb->GetPID(), PageSize);

    delete buffer;
    #endif

	printf("Loaded Program: %d code | %d data | %d bss\n",noffH.code.size,noffH.initData.size,noffH.uninitData.size);
}

AddrSpace::AddrSpace() {
}

AddrSpace::AddrSpace(TranslationEntry* table, int page_count, int oldPID) 
{
    pageTable = table;
    numPages = page_count;

    pcb = new PCB(pm->GetPID(), -1, NULL, -1);
    pm->AddProcess(pcb);

    #ifdef VM
    for (int i = 0; i < numPages; i++) 
    {
        if (pageTable[i].valid)
           vm->AddPage(&pageTable[i], pcb->GetPID(), machine->mainMemory + (pageTable[i].physicalPage * PageSize), PageSize);
        else
            vm->CopyPage(&pageTable[i], oldPID, pcb->GetPID());
       pageTable[i].valid = FALSE;
       pageTable[i].physicalPage = -1;
    }
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------
//AddrSpace::~AddrSpace()
//{
//   delete pageTable;
//   delete pcb;
//}
#ifdef VM
AddrSpace::~AddrSpace() {
    int i;

    // Delete any pages in physical memory
    for (i = 0; i < numPages; i++) 
    {
        if (pageTable[i].valid) 
            memory->clearPage(pageTable[i].physicalPage);
    }

    delete pageTable;
}
#else
AddrSpace::~AddrSpace() 
{
    for (int i = 0; i < numPages; i++) 
       memory->clearPage(pageTable[i].physicalPage);

    delete pageTable;
}
#endif


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
	for(int i = 0; i<NumTotalRegs; i++){
		progRegisters[i] = machine->ReadRegister(i);
	}
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
	for(int i = 0; i < NumTotalRegs; i++){
		machine->WriteRegister(i, progRegisters[i]);
	}
}

//---------------------------------------------------------------------
// AddrSpace::Translate()
//      takes a virtual address and tranlates it into a physical
//      address
//
//      return true if virtual address was valid, false otherwise
//
//      "vAddr" : virtual address to be translated
//      "physAddr" : the physical address to be stored
//---------------------------------------------------------------------
bool
AddrSpace::Translate(int vAddr, int *physAddr)
{
    if(vAddr < 0)
    {
        return false;
    }
    
    int vpn = vAddr / PageSize; // PageSize defined in machine.h
    if(vpn > numPages) // > or >= ???
        return false;
    else if(!pageTable[vpn].valid)
    {
	#ifdef VM
 	PageFaultHandler(vAddr);
	#else
        return false;
	#endif
    }

    int offset = vAddr % PageSize;

    *physAddr = pageTable[vpn].physicalPage * PageSize + offset;  // should it be ".physicalPage" ???
    /*
    if(writing)
       	pageTable[vpn].dirty = TRUE;	
    */ 
    // ^^^ add this?? ^^^

   #ifdef VM
   pageTable[vpn].use = TRUE;
   #endif

    return true;
}


#ifdef VM
// Return Value ??? some int
// vaddr = virtual address
// file = file to read into memory
// size = size of data to read
// fileAddr = location in file to start at
int AddrSpace::ReadFile(int vaddr, OpenFile* file, int size, int fileAddr) {
   int bytes_read = 0;
   int bytes_wrote;
   int phys_addr;
   int write_size;
   char* mem_buffer;
   char* buffer = new char[PageSize];
   
   char* out_buffer;
   
   DEBUG('q', "--------------------Start vaddr: %d end vaddr: %d\n", vaddr, vaddr + size);
               
   while (bytes_read < size) {
       int read = file->ReadAt(diskBuffer, PageSize - (vaddr % PageSize), fileAddr);
       fileAddr += read;
       DEBUG('q', "Bytes read: %d\n", read);
       bytes_read += read;
       DEBUG('q', "We have vaddr: %d, and PageSize: %d, VPN: %d\n", vaddr, PageSize, vaddr/PageSize);
       out_buffer = diskBuffer;
   
       if (vm->GetPage(&pageTable[vaddr/PageSize], pcb->GetPID(), buffer, PageSize)) {
           DEBUG('7', "Starting at %d, we should be able to write %d bytes\n", vaddr, PageSize - (vaddr % PageSize));
           memcpy(buffer + (vaddr % PageSize), diskBuffer, PageSize - (vaddr % PageSize));
           out_buffer = buffer;
       }

       vm->AddPage(&pageTable[vaddr/PageSize], pcb->GetPID(), out_buffer, PageSize);
       vaddr += read;
   }
   
   delete buffer;
}
#else
//---------------------------------------------------------------------
// AddrSpace::ReadFile()
//      loads the code and data segments into the translated memory
//      instead at positon 0 like the constructor already does
//
//      return the number of bytes actually read
//
//      "virtAddr" : beginning virtual address of the bytes to be read
//      "file" : file where bytes are from
//      "size" : amount of bytes to be read
//      "fileAddr" : the file address
//---------------------------------------------------------------------
int
AddrSpace::ReadFile(int virtAddr, OpenFile* file, int size, int fileAddr)
{
    char diskBuffer[size]; 	// stores the bytes read
    int availBytes;		// num bytes available to read
    int physAddr;
    
    // get the number of bytes that are actually there
    int actualSize = file->ReadAt(diskBuffer, size, fileAddr); // @@@ what if actual size > size ???
    int bytesLeft = actualSize;

    int bytesCopied = 0;

    // read the bytes in the file
    while(bytesLeft > 0)
    { 
        bool valid = Translate(virtAddr, &physAddr);
		if(valid < 0)
        {
            DEBUG('a',"virtual address could not be translated\n");
	    return -1;   // could not translate virtAddr... there is an issue
        }
        
        availBytes = PageSize - (physAddr % PageSize);
        
        // Be sure not to under or over flow the buffer during copy
        // Be sure no to write too much of the file into memory
        int bytesToRead = 0;
        if(bytesLeft < availBytes)
	    bytesToRead = bytesLeft;
        else
	    bytesToRead = availBytes;
	bcopy(diskBuffer+bytesCopied, &machine->mainMemory[physAddr], bytesToRead);
	
	// decrement the number of bytes left to read
	bytesLeft -= bytesToRead;
        // increment the number of bytes already read
	bytesCopied += bytesToRead;
	// increment to the next virtual address to write to
	virtAddr += bytesToRead;
    } 
    
    return actualSize;                
}
#endif


//Function to make a copy of an address space
AddrSpace*
AddrSpace::Duplicate(){
	if(numPages > memManager->getAvailable())
		return NULL;

	AddrSpace* dup = new AddrSpace();
	dup->numPages = this->numPages;
	dup->pageTable = new TranslationEntry[numPages];
	for (int i = 0; i < numPages; i++) {
	    dup->pageTable[i].virtualPage = i;
	    dup->pageTable[i].physicalPage = memManager->getPage();
	    dup->pageTable[i].valid = this->pageTable[i].valid;
	    dup->pageTable[i].use = this->pageTable[i].use;
	    dup->pageTable[i].dirty = this->pageTable[i].dirty;
	    dup->pageTable[i].readOnly = this->pageTable[i].readOnly;
	    bcopy(machine->mainMemory + this->pageTable[i].physicalPage * PageSize,
	                    machine->mainMemory + dup->pageTable[i].physicalPage
	                                    * PageSize, PageSize);
	}
	
	return dup;	
}

int
AddrSpace::getNumPages(){
	return numPages;
}


