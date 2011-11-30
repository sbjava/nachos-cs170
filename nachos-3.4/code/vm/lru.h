#ifndef LRU_H
#define LRU_H

#include "iReplacement.h"
#include "machine.h"
#include "synch.h"
//#include "system.h"


class LRU : public IReplacement {
    public:
        LRU();
        ~LRU();
        virtual void pageUsed( int pPage );
        virtual int getReplacement();
        virtual void clearPage( int pPage );
        virtual void markPage( int pPage );

		//added functions
		TranslationEntry* GetLRU();
		void RecentlyUsed(TranslationEntry* entry);
		void Remove_LRU_Page();
		int tail_pid();
		void AddPage(TranslationEntry* page,int pid);
		void RemovePage(SpaceId pid);

	protected:
		class LRU_Node {
			public:
		        LRU_Node(TranslationEntry* page) { value = page; next = NULL; }
		        LRU_Node* next;
		        LRU_Node* prev;
		        TranslationEntry* value;
		        int pid;
		};

    private:
        int queue[NumPhysPages];
		LRU_Node* FindPage(TranslationEntry* page, LRU_Node& prev_node);
		LRU_Node* get_tail();
		void replace_head(TranslationEntry* entry);
		void replace_head(LRU_Node* node);
		LRU_Node* head;
		LRU_Node* tail;
		Lock* LRU_lock;

};

#endif
