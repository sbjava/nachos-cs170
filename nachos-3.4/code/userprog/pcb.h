// pcb.h
//  Defines the PCB class
//	<- Stores necessary information about a process.
//		-PID
//		-parent PID
//		-Thread*
//		-thread status
		
#ifndef PCB_H_
#define PCB_H_

#define GOOD 0;
#define BAD 1;
#define RUNNING 2;
#define BLOCKED 3;

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