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
}


//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
//      Cleanup
//----------------------------------------------------------------------
MemoryManager::~MemoryManager ()
{
    delete pages;
}

//----------------------------------------------------------------------
// MemoryManager::getPage
//      Allocates the first clear page.
//----------------------------------------------------------------------
int
MemoryManager::getPage ()
{
    // find will return index of page found and Mark() the page
    int freePage = pages->Find();
    // if all pages are in use then return an invalid index
    if(freePage == -1)
	return -1;
    numUsedPages++;

    return freePage;
}

//----------------------------------------------------------------------
// MemoryManager::clearPage
//      Takes the index of page i and frees it.
//----------------------------------------------------------------------
void
MemoryManager::clearPage (int i)
{
    pages->Clear (i);
    numUsedPages--;
}

int
MemoryManager::getAvailable ()
{
    return NUM_OF_PHYSICAL_PAGES - numUsedPages;
}
        
