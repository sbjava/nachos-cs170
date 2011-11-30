#include "swapManager.h"
#include "system.h"
#include "machine.h"

SwapManager::SwapManager(){
    fileSystem->Create( "SWAP", SWAP_SIZE * PageSize );
    swapFile = fileSystem->Open( "SWAP" );
    swapMap = new BitMap( SWAP_SIZE );

	PIDs = new SpaceId[ SWAP_SIZE ];
	entries = new TranslationEntry*[ SWAP_SIZE];
}

SwapManager::~SwapManager(){
    fileSystem->Remove( "SWAP" );

    delete swapMap;
	delete PIDs;
	delete entries;
}

// allocate a new page from swap
int SwapManager::allocPage(){
    int page = swapMap->Find();
    ASSERT( page >= 0 );

    // zero out memory in case this page has garbage remaining
    bzero( diskBuffer, PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, page * PageSize );
    
    return page;
}

// write buffer into swap

bool SwapManager::writePage( int sPage, char* buffer, int size, int offset ){
    return swapFile->WriteAt( buffer, size, sPage * PageSize + offset );
}


// copy swap page into physical memory
void SwapManager::swap( int pPageNum, int sPageNum ){
    swapFile->ReadAt( diskBuffer, PageSize, sPageNum * PageSize );
    bcopy( diskBuffer, machine->mainMemory + pPageNum * PageSize, PageSize );    
}

// copy one page in swap to another in swap
void SwapManager::copy( int fromPage, int toPage ){
    swapFile->ReadAt( diskBuffer, PageSize, fromPage * PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, toPage * PageSize );
}

// clear this page in swap
void SwapManager::clearPage( int sPage ){
    bzero( diskBuffer, PageSize );
    swapFile->WriteAt( diskBuffer, PageSize, sPage * PageSize );
    swapMap->Clear( sPage );
}

// return the number of free swap pages
int SwapManager::getFreePageCount(){
    return swapMap->NumClear();
}


//ADDED FUNCS
bool SwapManager::addPage(TranslationEntry* page, SpaceId pid, char* buffer, int size) {

        DEBUG('q', "Adding a page to the swap file...with pid %d and vpn %d\n", pid, page->virtualPage);

        for (int i=0; i < SWAP_SIZE; i++) {
                if ((PIDs[i] == pid)&&(swapMap->Test(i))&&(entries[i]->virtualPage == page->virtualPage)) {
                        if (swapFile->WriteAt(buffer, size, PageSize*i) != PageSize) {
                                return false;
                        }
                        //DEBUG('q', "Found the page in swap already when adding...\n");
                        return true;
                }
        }

        int free = swapMap->Find();

        //DEBUG('q', "**********************free page is %d and adding the owner as %d\n",free,pid);
        if (free == -1) {
                printf("Out of virtual memory!\n");
                Exit(1);
                //return false;
        }

        entries[free] = page;
        PIDs[free] = pid;

        if (swapFile->WriteAt(buffer, size, PageSize*free) != PageSize) {
                return false;
        }
}

bool SwapManager::addPage(TranslationEntry* page, SpaceId pid, int size) {
        DEBUG('q', "Adding a page to the swap file...with pid %d and vpn %d\n", pid, page->virtualPage);
        for (int i=0; i < SWAP_SIZE; i++) {
                if ((PIDs[i] == pid)&&(swapMap->Test(i))&&(entries[i]->virtualPage == page->virtualPage)) {
                        return true;
                }
        }
        int free = swapMap->Find();
        if (free == -1) {
                printf("Out of virtual memory!\n");
                Exit(1);
                //return false;
        }

        entries[free] = page;
        PIDs[free] = pid;
}

void SwapManager::writePage(SpaceId pid, int vpn, char* buffer) {
        DEBUG('q', "Writing a page back to the swap file...\n");

        for (int i=0; i < SWAP_SIZE; i++) {
                if ((PIDs[i] == pid)&&(entries[i]->virtualPage == vpn)&&(swapMap->Test(i))) {
                        if (swapFile->WriteAt(buffer, PageSize, PageSize*i) != PageSize) {
                                DEBUG('q', "Error writing dirty page into swap file!\n");
                                ASSERT(false);
                        }                        
                        return;
                }
        }

        DEBUG('q', "Trying to write back a page not in the swap file. ERROR.\n");
        ASSERT(false);
}

void SwapManager::removePages(SpaceId pid) {
        DEBUG('q', "Removing a page from the swap file for PID %d...\n", pid);

        for (int i=0; i < SWAP_SIZE; i++) {
                if (PIDs[i] == pid) {
                        PIDs[i] = -1;
                        if (swapMap->Test(i)) {
                                swapMap->Clear(i);
                                entries[i] == NULL;
        DEBUG('q', "entries for page %d removed from the swap file for PID %d...\n",i, pid);
                        }
                }
        }
}

// ASSUMES BUFFER IS BIG ENOUGH TO HOLD A PAGE
TranslationEntry* SwapManager::findPage(SpaceId pid, int vpn, char* buffer) {
        DEBUG('q', "Finding a page in the swap file...with pid %d, vpn %d\n", pid, vpn);
//      DEBUG('q'," line 100 SwapManager.cc ->Swap size =% d",SWAPSIZE);
        for (int i=0; i < SWAP_SIZE; i++) {
                //DEBUG('q', "WHAT\n");
                //bool b = (PIDs[i] == pid);
                //DEBUG('q', "THE\n");
                //bool c = (entries[i]->virtualPage == vpn);
                //DEBUG('q', "HELL\n");
                //bool d = (bitmap->Test(i));
//              DEBUG('q'," i =%d\n",i);
                if ((PIDs[i] == pid)&&(swapMap->Test(i))&&(entries[i]->virtualPage == vpn)) {

                                        //DEBUG('q',"pageSize =%d\n",PageSize);
                        if (swapFile->ReadAt(buffer, PageSize, PageSize*i) != PageSize) {
                                // THROW AN ERROR
                                DEBUG('q'," throw error in Swapmanager.cc line 111\n");
                        }
//                                              DEBUG('q'," exiting this function findPage at line 116 and entries[%d]= %d",i,entries[i]);
                        return entries[i];
                }
        }

        DEBUG('q', "Page not found in swap file...\n");
        
        return NULL;
}


