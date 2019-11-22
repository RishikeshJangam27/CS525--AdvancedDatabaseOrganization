
/*
Authors:1.Lavanya Juvvala
	 2.B V SaiNaveena 
	 3.Rishikesh Jangam
*/

#include<stdio.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include <string.h>

FILE *file_p;  //Global variable pointing to current page pointer
extern void initStorageManager (void)
{
	file_p=NULL;
}
/*
	Function Name: createPageFile
	Input Arguments: char *fileName
	Description:
	1) Check if the file already exists
	2) If it is present, throw an error message that the file is already present.
	3) If it is not present, create a file and allocate size of one PAGE to it.	
*/
extern RC createPageFile(char *fileName)//to create new page
{
	int counter=0;        
	file_p=fopen(fileName,"wb"); //creating and opening a file in write mode
	if(file_p!=NULL)
	{
		for (counter=0;counter<PAGE_SIZE;counter++)
		{
			fputc(0,file_p);  //FIll the page  with '0' 
		}
		fclose(file_p);        //Close the file
		return RC_OK;         //returns message that file has been created
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}

}
/*
	Function Name: openPageFile
	Input Arguments: char *fileName, SM_FileHandle *fHandle
	Description:
	1) Check if the file with the provided file name exists.
   	2) If it does not exist, throw the error(RC_FILE_NOT_FOUND)
   	3) If it exists, check for the total number of pages that the file has.
   	4) After opening the file, initiate the structure elements needed.	
*/
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)//opening the file
{
	file_p=fopen(fileName,"r");//Opens the file in read mode
	if(file_p!=NULL)
	{
		fseek(file_p,0L,SEEK_END);
		long int curPos=ftell(file_p);
		fHandle->mgmtInfo=file_p;
		fHandle->totalNumPages=curPos/PAGE_SIZE;
		fHandle->curPagePos=0;
		fHandle->fileName=fileName;
		fseek(file_p,0L,SEEK_SET);
		return RC_OK; 
	}
	else
	{
		return RC_FILE_NOT_FOUND; 
	}
}
/*
	Function Name: closePageFile
	Input Arguments: SM_FileHandle *fHandle
	Description:
	1) Close the file and return a success message if the file could be located.
	2) If the file could not be located, return an error message.
*/
extern RC closePageFile(SM_FileHandle *fHandle)//closing the file
{
	int isclosed=fclose(file_p); 
	if(isclosed != 0 || file_p == NULL)
	//Throws an error message if the file is not closed or if the file is pointing to NULL
	{
		return RC_FILE_NOT_FOUND;      
	}
	else
	{
		file_p=NULL;  
        	return RC_OK;
	}
}
/*
	Function Name: destroyPageFile
	Input Arguments: char *fileName
	Description:
	1) Check if the file is present, and delete  the file.
   	2) If successful, return success message.
   	3) if failed, return a failure message.	
*/
extern RC destroyPageFile(char *fileName)//removing the  file from memory
{
	int isdestroyed=remove(fileName);
	if(isdestroyed != 0) //returns an error message if file is not removed
	{
		return RC_FILE_NOT_FOUND; 
	}
	
	else
	{
		return RC_OK;
	}
}
/*
	Function Name: readBlock
	Input Arguments: int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1)Check if the file already exists with help of file descriptor 
	2)Move the file descriptors position to the page requested in pageNum.
	3)Read the content of length 4096 bytes and load to the memory specified in mempage
*/
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) //Reading and storing content of block in memPage
{
	file_p = fopen(fHandle->fileName, "r"); 
	if(file_p != NULL)
        {
		if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	{
        		fclose(file_p);  
        		return RC_READ_NON_EXISTING_PAGE;
    		}

		fseek(file_p, (pageNum * PAGE_SIZE), SEEK_SET); //Setting the file pointer to the file start
	
		if(fread(memPage, 1, PAGE_SIZE, file_p) < PAGE_SIZE)
		
		//return RC_INSUFFICIENT_MEMORY;// Error message is displayed if the size of mempage is less than the size of the page to which file pointer points.
	    	fHandle->curPagePos = ftell(file_p);  
			//update the position of the file pointer
	    
			fclose(file_p);   
    		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND; //If File pointer points to a null value
	}
}
/*
	Function Name: getBlockPos
	Input Arguments: SM_FileHandle *fHandle
	Description:
	1)Check if the file exists
	2)Get the current block position 	
*/
extern int getBlockPos (SM_FileHandle *fHandle) //gives the block position
{
return fHandle->curPagePos;  //Gives the current block position
}
/*
	Function Name: readFirstBlock
	Input Arguments: SM_FileHandle *fHandle, SM_PageHandle memPage
*/
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) //reading the first block of file
{
	file_p = fopen(fHandle->fileName, "r"); 
	if(file_p != NULL)       
	{      
		printf("Position of the current page is :%d",fHandle->curPagePos);
		if(fHandle->curPagePos!=0)
		{
			fHandle->curPagePos=0;  //Assign current page position to the first block
		}
		fseek(file_p,0L,SEEK_SET);  //Read the first block into memPage
		if(fread(memPage, 1, PAGE_SIZE, file_p) < PAGE_SIZE)
			//return RC_INSUFFICIENT_MEMORY;    
		return RC_OK;       
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}	   
}
/*
	Function Name: readPreviousBlock
	Input Arguments: SM_FileHandle *fHandle,SM_PageHandle memPage
	Description:
	1)Check if the file already exists and it is not trying to access non existing page in the file
	2)Read the content to mempage of the previuous page position 
*/
extern RC readPreviousBlock (SM_FileHandle *fHandle,SM_PageHandle memPage) // Reading Previous Block of a File
{
	file_p = fopen(fHandle->fileName, "r");  
	if(file_p != NULL)
        {
		printf("Position of the current page :%d",fHandle->curPagePos);
		
		if(fHandle->curPagePos==0)
			return RC_READ_NON_EXISTING_PAGE;  
			// here pointer is pointing to first block but there is no previos block,so it returns error
		else
			fread(memPage, fHandle->curPagePos-PAGE_SIZE,1,file_p); //reading the previous block into mempage.
  		return RC_OK; 
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}
/*
	Function Name: readCurrentBlock
	Input Arguments: SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1)Check if the file already exists 
	2)Read the content to mempage of the current page position 
*/
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) 
/*Reading the Current Block of a File*/
{
	file_p = fopen(fHandle->fileName, "r"); //opening the file in read mode
	if(file_p != NULL)
        {
		printf("Current Page position is :%d",fHandle->curPagePos);
		fread(memPage, PAGE_SIZE,1, file_p);  //reading the current block into mempage
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}	

}
/*
	Function Name: readNextBlock
	Input Arguments: SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1)Check if the file already exists or trying to access non existing page 
	2)Read the content to mempage of the next page position	
*/
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)//Reading the Next Block of a File
{
	file_p = fopen(fHandle->fileName, "r"); //opens file in read mode
	if(file_p != NULL)
        {
		printf("Position of the current page is :%d",fHandle->curPagePos);
		if(fHandle->curPagePos==fHandle->totalNumPages)
			return RC_READ_NON_EXISTING_PAGE;
		else
			fread(memPage, fHandle->curPagePos+PAGE_SIZE,1, file_p);	
		return RC_OK;  
	}
	else
	{
		return RC_FILE_NOT_FOUND;
	}
}
/*
	Function Name: readLastBlock
	Input Arguments: SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1)Check if the file already exists
    	2)Move the file descriptor to the last page of the file
        3)Read the content to mempage.
*/

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)//reading the last block of a file
{
	file_p = fopen(fHandle->fileName, "r");  //opening the file in read mode
	if(file_p != NULL)
        {
		printf("Page position is :%d",fHandle->curPagePos);
		fseek(file_p,0L,SEEK_END); //sets the position of the pointer to the end of the file
		long int curPos=ftell(file_p);
		fHandle->curPagePos=curPos-PAGE_SIZE;  //sets the position of the pointer to the starting of the last block of  file
		fread(memPage,PAGE_SIZE,1, file_p); //Reads the last block to the mempage
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;  
	}
}
/*
	Function Name: writeBlock
	Input Arguments: int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1) Check if the file is available.
	2) Sets the file pointer to the given page number.	
	3) Write Contents to the file at the given page number.
	4) Close the file
*/
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)// Writing a page to disk at a given position.
{
	FILE *file_p;
	int currentPosition, currentPage, actualPosition;
	file_p = fopen(fHandle->fileName, "wb");  //Opening the file in write mode   
	if(file_p!=NULL)
	{      
		currentPosition = ftell(file_p);   //Getting the file pointer position
        	currentPage = currentPosition / 4096;  //Calculates the current position of the block			
		if (currentPage>pageNum)  //Current position of page is greater than the given page number 
		{       
			actualPosition=(currentPage*4096) - (4096 *(currentPage - pageNum));     
			fseek(file_p, actualPosition, SEEK_SET);              
			fwrite(memPage, 1, strlen(memPage), file_p);
	
		}
		else if (currentPage<pageNum) //Current position of page is less than the given page number 
		{
		   	actualPosition=(currentPage*4096) - (4096 *(currentPage + pageNum));  //Calculating Actual position of file pointer  
			fseek(file_p, actualPosition, SEEK_SET);    
			fwrite(memPage, 1, strlen(memPage), file_p);  	
		}
		else    
		{    
			fseek(file_p, pageNum * 4096, SEEK_SET); //Assigning Current page position to the specified page number        
			fwrite(memPage, 1, strlen(memPage), file_p);	
		}
		
		fclose(file_p);  
		return RC_OK;
	}
	else
	{
		return RC_FILE_NOT_FOUND;  
	}
	
}
/*
	Function Name: writeCurrentBlock
	Input Arguments: SM_FileHandle *fHandle, SM_PageHandle memPage
	Description:
	1) Opens the file in write mode.
	2) Check the current position of the pointer and start writing the block of data from that place.	
*/
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)//Write a page to disk at the current position.
{
	FILE *file_p;
	file_p = fopen(fHandle->fileName, "a");  
	int current_position;
	if(file_p!=NULL)
	{
		current_position = ftell(file_p);     // Acquire the position of pointer 
    }
	return writeBlock(current_position,fHandle,memPage);                    
			
}
/*
	Function Name: appendEmptyBlock
	Input Arguments: SM_FileHandle *fHandle
	Description:
	1) Opens the file in the append mode.
   	2) Checks the total pages and moves the pointer to the end of file.
  	3) Add a new page and fills it with '\0' in the empty block
*/
extern RC appendEmptyBlock(SM_FileHandle *fHandle)//Appending the empty block to the file
{
	FILE *file_p;
	file_p = fopen(fHandle->fileName, "a"); 
	if(file_p!= NULL)     
	{      
		int Result = fseek(file_p, 0, SEEK_END); // Setting the pointer at the position of last block            

		if (Result!= 0)   
		{
			return RC_FILE_NOT_FOUND; 
		}
		else
		{    
			fHandle->totalNumPages++;    // Addition of an empty block
				  
			fclose(file_p);    
			return RC_OK;
		}
	}
	else
	{
		return RC_FILE_NOT_FOUND; 
	}
		
}
/*
	Function Name: ensureCapacity
	Input Arguments: int numberOfPages, SM_FileHandle *fHandle
	Description:
	1) Find the sppecified file,if it fails return an appropriate error message if not, continue.
	2) Calculate the number of pages that the file has.
	3) If the file's memory is not sufficient, calculate the memory needed so that the file has required capacity.

*/
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) //increasing the file size if number of pages are less
{
	FILE *file_p;
	file_p = fopen(fHandle->fileName, "a");                          
	if (file_p!= NULL)
	{                 
		if (fHandle->totalNumPages < numberOfPages) 
        { 
			fHandle->totalNumPages = numberOfPages; // updating the file capacity
        	}
		fclose(file_p); 
		return RC_OK;
	}
	else
		return RC_FILE_NOT_FOUND; 
	
}
