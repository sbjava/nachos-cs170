// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
#ifdef FILESYS
	if (fileSize> MaxFileSize){
		printf("Unable to save file, bigger than max file size %d\n",MaxFileSize);
		return false;
	}
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
	//@@@
	numSectors += ((numSectors - 4) + PointerSectors - 1)/PointerSectors;

    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
	
	
	int dir = numSectors;
	
	if(numSectors > 4){
		dir = 4;
	}
	
	for(int i = 0; i < dir; i++){
		directDataSectors[i] = freeMap->Find();
	}
	
	for(int j = 0; j < ((numSectors - 4) + PointerSectors - 1)/PointerSectors; j++){
		indirectDataSectors[j] = new IndirectPointerBlock();
		pointerBlockSectors[j] = freeMap->Find();
		int sect = PointerSectors;
		
		if (j == ((numSectors - 4) + PointerSectors - 1) - 1) {
			sect = (numSectors - 4) - PointerSectors*j;
		}

		for (int k = 0; k < sect; k++) {
			indirectDataSectors[j]->PutSector(freeMap->Find());
		}
	}

	/*
    int i = 0;
    int sectorsToAllocate = numSectors;

    
    DEBUG('f', "Allocating %d sectors for a file size of %d. Sector Size:%d. Max Direct:Indirect %d:%d. Sectors/Indirect %d. Max File Size:%d\n",  numSectors, fileSize, SectorSize, NumDirect,NumInDirect,MaxIndirectPointers,MaxFileSize);
    
    while(sectorsToAllocate > 0){
	if (i < NumDirect){
	  dataSectors[i] = freeMap->Find();
	  sectorsToAllocate--;
	  DEBUG('f', "\nAllocating new direct dataSector[%d]:%d\n" , (i),dataSectors[i]);
	  
	}
	else{
	  indirectPointers[i-NumDirect] = new IndirectPointerBlock();
	  indirectSector[i-NumDirect] = freeMap->Find();
	  DEBUG('f', "\nAllocating new indirectPointerBlock[%d]:%d\n" , (i-NumDirect),indirectSector[i-NumDirect]);
	  int sectorsToAddToThisPage = sectorsToAllocate;
	  if (sectorsToAddToThisPage > MaxIndirectPointers)
	    sectorsToAddToThisPage = MaxIndirectPointers;
	  for (int j = 0 ; j < sectorsToAddToThisPage; j++){
	  
	    //DEBUG('f', "Putting a sector to indirectPointer[%d].\n" , (i-NumDirect));
	    int newSec = freeMap->Find();
	    
	    DEBUG('f', "-%d",newSec );
	    indirectPointers[i-NumDirect]->PutSector(newSec);
	    sectorsToAllocate--;
	  }
	  
	}      
     
      i++;
    }
    DEBUG('f', "\nReturning True for filehdr alloc \n" );
	    */
    return TRUE;
    
#else
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
		return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->Find();
	 DEBUG('f', "\nReturning True for filehdr alloc \n" );
    return TRUE;
#endif
    
    
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{


    
    #ifdef FILESYS        
    int indirects = ((numSectors - 4) + PointerSectors - 1)/PointerSectors;
    int directs = (numSectors>4)? 4 : numSectors;

    for (int i = 0; i < directs; i++) {
	ASSERT(freeMap->Test((int) directDataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) directDataSectors[i]);
    }

    

    for (int j = 0; j < indirects; j++) {
	ASSERT(freeMap->Test((int) pointerBlockSectors[j]));  // ought to be marked!
	freeMap->Clear((int) pointerBlockSectors[j]);
        indirectDataSectors[j]->Deallocate(freeMap);
	delete indirectDataSectors[j];
    }
	#else
	for (int i = 0; i < numSectors; i++) {
        DEBUG('f',"Delloc %d (%d)\n",numSectors, NumDirect);
        //if (numSectors < NumDirect){
            ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
            freeMap->Clear((int) dataSectors[i]);
       //}
    }
	#endif
	
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    
//#ifdef FILESYS
//	char* buf = new char[SectorSize];
//	bzero(buf,SectorSize);
//	synchDisk->ReadSector(sector, buf);
//	bcopy( buf,(char *) this, HdrSize);
//	DEBUG('f',"FileHdr fetched num Sectors:%d at sec:%d\n",numSectors,sector);
//	if (numSectors > NumDirect){
//        int indirects = divRoundUp((numSectors - NumDirect),PointersPerIndirect);
//      	for(int i = 0; i < indirects; i++){
//	    	DEBUG('f',"FileHdr fetch numSectors:%d numIndirects:%d\n",numSectors, indirects);
//
//			if (indirectSector[i] != 0){
//				DEBUG('f',"^^FileHdr fetchin IndirectPointer[%d] at sector:%d\n",i,indirectSector[i]);
//				indirectPointers[i] = new IndirectPointerBlock();
//				indirectPointers[i]->FetchFrom(indirectSector[i]);
//			}
//      }
//    }
//#else
	synchDisk->ReadSector(sector, (char *)this);      
      
//#endif
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    
/*#ifdef FILESYS
	char* buf = new char[SectorSize];
	bzero(buf, SectorSize);
	int t = HdrSize;
	bcopy((char *) this, buf, HdrSize);
	synchDisk->WriteSector(sector, buf); 
	
	DEBUG('f',"FileHdr writeBack num Sectors:%d at sec:%d\n",numSectors,sector);
    if (numSectors > NumDirect){

        int indirects = divRoundUp((numSectors - NumDirect),PointersPerIndirect);
    	
      	for(int i = 0; i < indirects; i++){
	
		DEBUG('f',"^^FileHdr Writing back IndirectPointer[%d] at sector:%d\n",i,indirectSector[i]);
		if (indirectSector[i] != 0)
			indirectPointers[i]->WriteBack(indirectSector[i]);
      }
    }
#else */      
    synchDisk->WriteSector(sector, (char *)this);      
//#endif
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
  #ifdef FILESYS
	int indirects = ((numSectors - 4) + PointerSectors - 1)/PointerSectors;
    int sector = offset / SectorSize;
	
    if (sector < 4) {
		return (directDataSectors[offset / SectorSize]);
    }
    else {
	    return indirectDataSectors[(sector - 4)/SectorSize]->ByteToSector(offset - 4*SectorSize - ((sector - 4)/SectorSize) * SectorSize);
    }
   /* DEBUG('f',"BytoSector :%d : ",offset);
    
    int sector = offset/ SectorSize;
    if (sector < NumDirect){
      DEBUG('f',"Direct Sector offset %d, is at sector: %d\n",sector,dataSectors[sector]);     
      return dataSectors[sector];
    }
    else{
      int relSec = divRoundDown((numSectors - NumDirect),PointersPerIndirect);
      int newOff = offset - SectorSize * (NumDirect);
      
      DEBUG('f',"Indirect Sector offset %d, with newoffset is at: is at %d : \n",relSec,newOff);
      DEBUG('f'," found at sector: %d\n",indirectPointers[relSec]->ByteToSector(newOff));
      return indirectPointers[relSec]->ByteToSector(newOff);
      
    }*/
  #else  
    //return(dataSectors[offset / SectorSize]);
	return(directDataSectors[offset / SectorSize]);
  #endif
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
	#ifdef FILESYS
		int i, j, k;
    char *data = new char[SectorSize];
	
    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    for (i = 0; i < numSectors; i++)
		printf("%d ", directDataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
		synchDisk->ReadSector(directDataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
			if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
				printf("%c", data[j]);
            else
				printf("\\%x", (unsigned char)data[j]);
		}
        printf("\n"); 
    }
    delete [] data;
	#else
   		int i, j, k;
   		char *data = new char[SectorSize];
   		
   		printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
   		for (i = 0; i < numSectors; i++)
   		printf("%d ", dataSectors[i]);
   		printf("\nFile contents:\n");
   		for (i = k = 0; i < numSectors; i++) {
   		synchDisk->ReadSector(dataSectors[i], data);
   		    for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
   		    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
   			printf("%c", data[j]);
   		        else
   			printf("\\%x", (unsigned char)data[j]);
   		}
   		    printf("\n"); 
   		}
   		delete [] data;
	#endif
}

void FileHeader::setNumBytes(int newBytes){
	numBytes= newBytes;
}

#ifdef FILESYS
bool 
FileHeader::
ExtendFile
(BitMap *freeMap, int n, int bytes) {
	//   is n the number of sectors?
	//   numBytes += n;
	//   int offset = numBytes - divRoundDown(numBytes, SectorSize)*SectorSize;
	//   int newSectors  = divRoundUp(n - (SectorSize - offset), SectorSize);
	//   newSectors += ((newSectors - 4) + PointerSectors - 1)/PointerSectors;
    if (freeMap->NumClear() < n) 
	{
		printf("Out of disk drive space!\n");
		return FALSE;		// not enough space
    }

   
    if (numSectors < 4) 
	{
		int max = n + numSectors;	

		if (max > 4) 
		{
           max = 4;
		}

        for (int i=numSectors; i < max; i++) 
		{
			directDataSectors[i] = freeMap->Find();
        }

        n = n - (max - numSectors);
        numSectors = numSectors + (max - numSectors);
    }

    numBytes = bytes;

    
    if (n == 0) {
       return true;
    }

    int existing_blocks = divRoundUp((numSectors - 4), PointerSectors);
    int block_space = existing_blocks*PointerSectors - (numSectors - 4);



    if (block_space - n > 0) {
       block_space = n;
    }

    n -= block_space;

    if (n == 0) 
	{
       return true;
    }

    int new_blocks = divRoundUp(n, PointerSectors);

    
    for (int j=existing_blocks; j < existing_blocks + new_blocks; j++) 
	{
		indirectDataSectors[j] = new IndirectPointerBlock();

		if (j == existing_blocks + new_blocks - 1) 
		{
			int remains = n;

			for (int k=0; k < remains; k++) {
				indirectDataSectors[j]->PutSector(freeMap->Find());
				n--;
			}

		}
         else 
		 {
             for (int k=0; k < PointerSectors; k++) 
			 {
                 indirectDataSectors[j]->PutSector(freeMap->Find());
				 n--;
             }
         }
    }

    if (n == 0) 
	{
       return true;
    }

    return false;

	
}
#endif

//bool FileHeader::ExtendFile( int sectorsToAllocate){
//    DEBUG('f',"Extending file by %d sectors\n",sectorsToAllocate);
//    sectorsToAllocate -= numSectors;
//	if (sectorsToAllocate <= 0)
//		return false;
//		
//	DEBUG('f', "opening free \n");
//	OpenFile* freeMapFile = new OpenFile(FreeMapSector);
//    BitMap* freeMap = new BitMap(NumSectors);
//    DEBUG('f', "Fetching free Map\n");
//    freeMap->FetchFrom(freeMapFile);
//    
//    if (freeMap->NumClear() < sectorsToAllocate)
//        return false;
//
//	int i = numSectors;	
//	while(sectorsToAllocate > 0){
//	    if (i < NumDirect){
//	      dataSectors[i] = freeMap->Find();
//	      DEBUG('f', "\nAllocating new direct DataSector[%d]:%d\n" , (i),dataSectors[i]);
//	      
//	      sectorsToAllocate--;
//	      numSectors++;
//	    }
//	    else{
//	      indirectPointers[i-NumDirect] = new IndirectPointerBlock();
//	      DEBUG('f', "\nAllocating new indirectPointerBlock[%d]:" , (i-NumDirect));
//	      int sectorsToAddToThisPage = sectorsToAllocate;
//	      if (sectorsToAddToThisPage > MaxIndirectPointers)
//		    sectorsToAddToThisPage = MaxIndirectPointers;
//	      for (int j = 0 ; j < sectorsToAddToThisPage; j++){
//	      
//		    DEBUG('f', "." );
//		    //DEBUG('f', "Putting a sector to indirectPointer[%d].\n" , (i-NumDirect));
//		    int newSec = freeMap->Find();
//		    indirectPointers[i-NumDirect]->PutSector(newSec);
//		    sectorsToAllocate--;
//		    numSectors++;
//	      }
//	    }
//      
//	}   
//	
//	freeMap->WriteBack(freeMapFile);
//	delete freeMap;
//	delete freeMapFile;
//    return true;
//}
