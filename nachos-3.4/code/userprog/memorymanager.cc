// memorymanager.cc 
//      Memory Manager used to facilitate contiguous virtual memory

#include "memorymanager.h"

//----------------------------------------------------------------------
// MemoryManager::MemoryManager
//      Create a memory manager
//----------------------------------------------------------------------
MemoryManager::MemoryManager ()
{
    pages = new BitMap (NUM_OF_PHYSICAL_PAGES);
	memLock = new Lock("mem lock");
}


//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//      Cleanup
//----------------------------------------------------------------------
MemoryManager::~MemoryManager ()
{
    delete pages;
	delete memLock;
}

//----------------------------------------------------------------------
// MemoryManager::getPage
//      Allocates the first clear page.
//----------------------------------------------------------------------
int
MemoryManager::getPage ()
{
	///@@@ memlOCk
	memLock->Acquire();
    // find will return index of page found and Mark() the page
    int freePage = pages->Find();
    // if all pages are in use then return an invalid index
    if(freePage == -1)
		return -1;

    numUsedPages++;
	
	//@@@ memLock added
	memLock->Release();
	
    return freePage;
}

//----------------------------------------------------------------------
// MemoryManager::clearPage
//      Takes the index of page i and frees it.
//----------------------------------------------------------------------
void
MemoryManager::clearPage (int i)
{
	//@@@ acquire
	memLock->Acquire();
	
    pages->Clear(i);
    numUsedPages--;
	
	//@@@ memLock added
	memLock->Release();
}

int
MemoryManager::getAvailable ()
{
	//@@@ acquire
	memLock->Acquire();
	
	int tmp = pages->NumClear();
	
	//@@@ memLock added
	memLock->Release();
	
	return tmp;	
}
        
