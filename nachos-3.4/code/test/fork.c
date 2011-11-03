#include "syscall.h"

void test_func1() {
	//DEBUG('t', "Forked. Entering test_func1");    
	Halt();
}


void 
main() {
    Fork(test_func1);
}
