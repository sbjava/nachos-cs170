// SysOpenFile.h

#ifndef SYSOPENFILE_H_
#define SYSOPENFILE_H_
#define SOFILETABLE_SIZE 100
#include "filesys.h"
#include "bitmap.h"
#include "synch.h"
// The following class defines a "SysOpenFile"

class SysOpenFile{
public:
    OpenFile *file; 	// a pointer to the file system's OpenFile object
    char * fileName;    // file name and the number of user processes accessing
    int fileID;
    int numUsersAccess;	// Number of user processes accessing


	//@@@ BitMap usedFiles;
	
 	//@@@ void closeOne();	//close a SysOpenFile and accessing processes
    //@@@ void close();   //close all SysOpenFile and accessing processes  
	Lock* lock;
	
	SysOpenFile();
	SysOpenFile(char* name, OpenFile* f, int id);
	~SysOpenFile();               
};
#endif /* SYSOPENFILE_H_ */

