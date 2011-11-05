// pcb.h
//  Defines the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#ifndef PCB_H_
#define PCB_H_

class Thread;

class PCB {
public:
	int pid;
	int parentPid;
	Thread *thread;
	PCB();
	~PCB();
};

#endif