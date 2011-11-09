// SysOpenFile.h

#ifndef SYSOPENFILE_H_
#define SYSOPENFILE_H_
#define SOFILETABLE_SIZE 100
#include "filesys.h"
#include "bitmap.h"

// The following class defines a "SysOpenFile"

class SysOpenFile{
public:
    OpenFile *file; 	// a pointer to the file system's OpenFile object
    char * fileName;    // file name and the number of user processes accessing
    int fileID;
    int numUsersAccess;	// Number of user processes accessing
	BitMap usedFiles;
    void closeOne();	//close a SysOpenFile and accessing processes
    void close();   //close all SysOpenFile and accessing processes  
                    
};
/*
// The following class defines a "SysOpenFileManager"
class SysOpenFileManager{
public:
    SysOpenFileManager();   // Initialize a SysOpenFileManager
    ~SysOpenFileManager();  // De-allocate a SysOpenFileManager
    int Add(SysOpenFile newSysOpenFile); //Add a SysOpenFile to the array
    SysOpenFile* Get(char* fileName,int& index); // Get SysOpenFile by "fileName"
    SysOpenFile* Get(int index);    // Get SysOpenFile by "index" of the array
    void closeOne(int index);       // Close a SysOpenFile by "index"
private:
    SysOpenFile sysOpenFileTable[SOFILETABLE_SIZE]; // array for all SysOpenFile
    BitMap bitMap;  // a BitMap for SysOpenFile manager methods

};
*/
#endif /* SYSOPENFILE_H_ */

