#ifndef VM_MANAGER_H
#define VM_MANAGER_H

#include "swapManager.h"
#include "iReplacement.h"
#include "translate.h"
#include "machine.h"
#include "syscall.h"
#include "lru.h"

class VMManager{
    public:
        VMManager();
        ~VMManager();

        int getPage();
        bool writePage( int pPage, char* buffer, int size, int offset );
        int swap( TranslationEntry* page );
        int getPhysicalPage( TranslationEntry* page );
        void clearPage( int sPage );
        int getFreePageCount();
		void copy( int fromPage, int toPage );
        void markPage( int pPage );
		
		//added funcs
		bool AddPage(TranslationEntry* page, SpaceId pid, char* buffer, int size);
		bool AddPage(TranslationEntry* page, SpaceId pid,int size);
        bool GetPage(TranslationEntry* page, SpaceId pid, char* buffer, int size);
		void Swap(int vpn, SpaceId pid);
		void CopyPage(TranslationEntry* page, SpaceId oldPID, SpaceId newPID);
		void RemovePages(SpaceId pid);
		void Mark(TranslationEntry* page);
				
		int free() {return swapMgr->free();}
		
    private:
        SwapManager* swapMgr;
        LRU* policy;
        int activePageMap[ NumPhysPages ];
        TranslationEntry* activePages[ NumPhysPages ];
};

#endif
