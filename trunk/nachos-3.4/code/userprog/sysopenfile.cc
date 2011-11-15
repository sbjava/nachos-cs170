// sysopenfile.cc
//     Defines SysOpenFile class that contains a pointer to the file 
//     system's OpenFile object and (char *)fileName for that file and 
//     the number of user processes currently accessing it. Declare an 
//     array of SysOpenFile objects for use by all system calls implemented.

#include "sysopenfile.h"

//----------------------------------------------------------------------
// SysOpenFile::closeOne
//      close a SysOpenFile and accessing processes
//----------------------------------------------------------------------

void SysOpenFile::closeOne() 
{
    if(numUsersAccess <= 0)
		return;
    numUsersAccess--;
    if (numUsersAccess == 0) 
    {
        delete fileName;
        delete file;
    }
}
//----------------------------------------------------------------------
// SysOpenFile::close
//      close all SysOpenFile and accessing processes
//----------------------------------------------------------------------
void SysOpenFile::close() 
{
    if (numUsersAccess <= 0)
	return;
    numUsersAccess = 0;
    delete fileName;
    delete file;
}

SysOpenFile::SysOpenFile():usedFiles(SOFILETABLE_SIZE)
{
	file = 0; 	
    fileName = 0;   
    numUsersAccess = 0;	
	//usedFiles = 0;	
}

SysOpenFile::~SysOpenFile()
{
	
}
