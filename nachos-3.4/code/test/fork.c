#include "syscall.h"

void test_func1() {
	//DEBUG('t', "Forked. Entering test_func1");    
	//Halt();
	int i = 0;
	while(i < 10)
		i++;
		
	Halt();	
}


void 
main() {
    Fork(test_func1);
}
