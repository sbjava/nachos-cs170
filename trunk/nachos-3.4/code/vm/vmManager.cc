#include "vmManager.h"
#include "swapManager.h"
#include "system.h"
#include "fifo.h"
#include "lru.h"

VMManager::VMManager(){
    swapMgr = new SwapManager();
//    policy = new FIFO();
//    policy = new LRU();
    
     policy = new LRU();

    // no pages in physical memory
    for( int i = 0; i < NumPhysPages; i++ ){
        activePageMap[i] = -1;
		activePages[i] = NULL;
    }

    //activePages = { 0 };
}

VMManager::~VMManager(){
    delete swapMgr;
    delete policy;
}

// allocates a swap page
int VMManager::getPage(){
    return swapMgr->allocPage();
}

// writes to swap
bool VMManager::writePage( int pPage, char* buffer, int size, int offset){
   	return swapMgr->writePage( pPage, buffer, size, offset );
}

// swaps a swap page into physical memory, clearing room and writing 
// victim to swap if necessary.
int VMManager::swap( TranslationEntry* newPage ){
    // get page number to load
    int pPage;
    int sPage = newPage->physicalPage;
    int oldPage = -1;
   


    //Part of code is deleted here.
 

    // swap new page in

    DEBUG( 'v', "Swapping out page %d, swapping in swap page %d to physical page %d\n",
           oldPage, sPage, pPage );
    swapMgr->swap( pPage, sPage );
    activePageMap[ pPage ] = sPage;
    activePages[ pPage ] = newPage;
    
	printf("L [%d]: [%d] -> [%d]\n", currentThread->space->pcb->pid, oldPage, pPage);

    return pPage;
}

void VMManager::copy( int fromPage, int toPage ){
    swapMgr->copy( fromPage, toPage );
}

// translate a swap page into a physical page and swaps a non-active page in.
int VMManager::getPhysicalPage( TranslationEntry* page ){
    int sPage = page->physicalPage;
    for( int i = 0; i < NumPhysPages; i++ ){
        if( activePageMap[i] == sPage ) return i;
    }
    // the page is not active, so swap
    return swap( page );
}

// remove page from active map, clear in swap mgr
void VMManager::clearPage( int sPage ){
    int pPage;
    DEBUG( 'v', "Clearing page %d\n", pPage );
    for( pPage = 0; pPage < NumPhysPages; pPage++ ){
        if( activePageMap[ pPage ] == sPage ){
            activePageMap[ pPage ] = -1;
            activePages[ pPage ] = 0;
            break;
        }
    }
    memManager->clearPage( pPage );
    swapMgr->clearPage( sPage );
    policy->clearPage( pPage );
}

// return the number of free swap sectors
int VMManager::getFreePageCount(){
    return swapMgr->getFreePageCount();
}

void VMManager::markPage( int pPage ){
    policy->markPage( pPage );
}



//added funcs
/*
bool AddPage(TranslationEntry* page, SpaceId pid, char* buffer, int size);
bool AddPage(TranslationEntry* page, SpaceId pid,int size);
bool GetPage(TranslationEntry* page, SpaceId pid, char* buffer, int size);

void CopyPage(TranslationEntry* page, SpaceId oldPID, SpaceId newPID);
void RemovePages(SpaceId pid);
void Mark(TranslationEntry* page);
		
int free() {swapMgr->free();}
*/
bool VMManager::AddPage(TranslationEntry* page, SpaceId pid, char* buffer, int size) {
        printf("Z %d: %d\n", pid, page->virtualPage);
        return swapMgr->addPage(page, pid, buffer, size);
}

bool VMManager::AddPage(TranslationEntry* page, SpaceId pid, int size) {
        printf("Z %d: %d\n", pid, page->virtualPage);
        return swapMgr->addPage(page, pid,size);
}

void VMManager::RemovePages(SpaceId pid) {
        //lru->RemovePage(pid);
		policy->RemovePage(pid);
        swapMgr->removePages(pid);
}

bool VMManager::GetPage(TranslationEntry* page, SpaceId pid, char* buffer, int size) {
    DEBUG('3', "in getPage.. pid: %i\n", pid);
	if (swapMgr->findPage(pid, page->virtualPage, buffer) != NULL) {
                return true;
    }

        return false;
}

void VMManager::CopyPage(TranslationEntry* page, SpaceId oldPID, SpaceId newPID) {
        char buffer[PageSize];
		DEBUG('3', "in copy page.. old pid: %i     new pid: %i\n", oldPID, newPID);
        swapMgr->findPage(oldPID, page->virtualPage, buffer);
        swapMgr->addPage(page, newPID, buffer, PageSize);
}

void VMManager::Swap(int vpn, SpaceId pid) {
        int proc_kill;
        TranslationEntry* page = new TranslationEntry();
        // No swap necessary, we have enough memory available
        //if (memory->freePages() != 0) {
		if(memManager->getAvailable() != 0){
                int phys_page = memManager->getPage();
                DEBUG('q', "Swapping into main memory page %d\n", phys_page);
                char* paddr = machine->mainMemory + phys_page*PageSize;
                page = swapMgr->findPage(pid, vpn, paddr);
				//DEBUG('q', "In vm->swap: phys_page %i \n", phys_page);			
				page->physicalPage = phys_page;
				//DEBUG('q', "In vm->swap: vpn%i   pid%i\n", vpn, pid);
				 

                //lru->AddPage(page,pid);
				policy->AddPage(page, pid);
				printf("L %d: %d -> %d\n", pid, vpn, phys_page);
                ///DEBUG('3', "L %d: %d -> %d\n", pid, vpn, phys_page);
        }
        else {
                // Assumption is, if its in main memory,
                // its in the swap file as well
                
				//proc_kill = lru->tail_pid();
				proc_kill = policy->tail_pid();

				//TranslationEntry* victim = lru->GetLRU();
				TranslationEntry* victim = policy->GetLRU();
				
                DEBUG('q', "Finding a victim to swap out of main memory page %d\n", victim->physicalPage);
                victim->valid = false;
                char* paddr = machine->mainMemory + victim->physicalPage*PageSize;
                if (victim->dirty) {
                        //DEBUG('3', "S %d: %d\n", pid, victim->physicalPage);
					printf("S %d: %d\n", pid, victim->physicalPage);
                        swapMgr->writePage(pid, victim->virtualPage, paddr);
                }
                else {
                    //DEBUG('3', "E %d: %d\n", pid, victim->physicalPage);
					printf("E %d: %d\n", pid, victim->physicalPage);
                }
                page = swapMgr->findPage(pid, vpn, paddr);
                page->physicalPage = victim->physicalPage;

                //lru->AddPage(page,pid);
                //lru->Remove_LRU_Page();
				policy->AddPage(page, pid);
				policy->Remove_LRU_Page();
        }

        page->valid = true;
        page->dirty = false;
}


