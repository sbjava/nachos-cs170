/*
	UserOpenfile.h
	Created on November 8th, 2011

*/

#ifndef USEROPENFILE_H
#define USEROPENFILE_H

class UserOpenFile{
public:
	char* fileName;
    int offset;
    int indexPosition;
	UserOpenFile();
	~UserOpenFile();
};

#endif