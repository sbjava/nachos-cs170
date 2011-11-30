#ifndef SWAP_MANAGER_H
#define SWAP_MANAGER_H

#include "openfile.h"
#include "bitmap.h"
#include "syscall.h"
#include "translate.h"

#define SWAP_SIZE 512

class SwapManager{
    public:
        SwapManager();
        ~SwapManager();

        int allocPage();
        bool writePage( int sPage, char* buffer, int size, int offset );		
		void swap( int pPageNum, int sPageNum );
        void copy( int fromPage, int toPage );
        void clearPage( int sPage );
        int getFreePageCount();

		//added funcs
		bool addPage(TranslationEntry* page, SpaceId pid, char* buffer, int size);
        bool addPage(TranslationEntry* page, SpaceId pid,int size);
        void removePages(SpaceId pid);
		TranslationEntry* findPage(SpaceId pid, int vpn, char* buffer);
		void writePage(SpaceId pid, int vpn, char* buffer);
		int free() {return swapMap->NumClear(); }

    private:
        OpenFile* swapFile;
        BitMap* swapMap;
		TranslationEntry** entries;
		SpaceId* PIDs;
};


#endif
