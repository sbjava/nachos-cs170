#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"

#define NUM_OF_PHYSICAL_PAGES 32

class MemoryManager
{
  public:
    MemoryManager();
    ~MemoryManager();

    int getPage();	// allocates the first clear page
    void clearPage(int i);		// takes index of a page and frees it
    int getAvailable();

  private:
    BitMap *pages;  			// size of NUM_OF_PHYICAL_PAGES
    int numUsedPages;			// numnber of the used pages
};

#endif
