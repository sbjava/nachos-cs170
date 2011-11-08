// pcb.h
//  Defines the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#ifndef PCB_H_
#define PCB_H_

#define PCB_GOOD 0;
#define PCB_BAD 1;
#define PCB_RUNNING 2;
#define PCB_BLOCKED 3;

class Thread;

class PCB {
public:
	int pid;
	int parentPid;
	Thread *thread;
	int status;
	PCB();
	~PCB();
};

#endif