//lru.cc

#include "lru.h"
#include "system.h"

LRU::LRU()
{
	tail = NULL;
	head = NULL;
	LRU_lock = new Lock("lru semaphore");
}

LRU::~LRU()
{
	
}

void LRU::pageUsed(int pPage)
{
	
}

int LRU::getReplacement()
{
	
}

void LRU::clearPage(int pPage)
{
	
}

void LRU::markPage(int pPage)
{
	
}



/*
class LRU : public IReplacement {
    public:
        LRU();
        ~LRU();
        virtual void pageUsed( int pPage );
        virtual int getReplacement();
        virtual void clearPage( int pPage );
        virtual void markPage( int pPage );

    private:
        int queue[NumPhysPages];
};
*/

//added functions
void LRU::AddPage(TranslationEntry* page, int pid) {
        LRU_lock->Acquire();
        LRU_Node* new_node = new LRU_Node(page);
        /* ADDED */
        new_node->pid =pid;
        replace_head(new_node);
        LRU_lock->Release();
}

void LRU::RemovePage(SpaceId pid) {
        LRU_lock->Acquire();
        LRU_Node *prev,*next,*current;
        current = head;
        //LRU_Node* temp = tail;
        //if (head->value == page) {
        //      return;
        //}
        
        while (current != NULL) {
                if (current->pid == (int) pid) {
                        memManager->clearPage(current->value->physicalPage);
                        prev =current->prev;
                        next = current->next;

                        if (prev != NULL) {
                                prev->next = current->next;
//                              current->next->prev = prev;
                        }
                        if(next!=NULL)
                        {
                                next->prev =current->prev;
                        }
                        if(head ==current)
                        {
                                head =current->next;
                        }
                        if( tail =current) 
                        {
                                tail =current->next;
                        }
                        //current->next = head;
                        //head->prev = current;
                        //head = current;
                        delete current;
                        current =next;
                        //return;
                }else
                {               
                //prev = current;
                current = current->next;
                }
        }
        LRU_lock->Release();
}
void LRU::Remove_LRU_Page()
{
        LRU_Node* temp =tail;
        tail = tail->prev;
        tail->next= NULL;
        delete (temp);
}
int LRU::tail_pid()
{
        return tail->pid;
}

void LRU::replace_head(LRU_Node* node) {
        LRU_lock->Acquire();
        LRU_Node* prev = NULL;
        LRU_Node* current = head;
        if(head ==NULL)
        {
                head = node;
                tail = node;
                node->next = NULL;
                node->prev =NULL;
                //tail = head ;
                return;
        }
        if (head == node) {
                return;
        }
        int i=0;
        while (current != NULL) {
                if (current == node) {
                        if (prev != NULL) {
                                prev->next = current->next;
                                //current->next->prev = prev;
                        }
                        if(current->next!=NULL)
                        {
                                current->next->prev =current->prev;
                        }
                        current->next = head;
                        head->prev = current;
                        head = current;
                        return;
                }
                
                prev = current;
                current = current->next;
        }

        node->next = head;
        node->prev = NULL;
        head->prev = node;
        head = node;
        LRU_lock->Release();
}

LRU::LRU_Node* LRU::FindPage(TranslationEntry* page, LRU_Node& prev_node) {

}

TranslationEntry* LRU::GetLRU() {
        return tail->value;
}
