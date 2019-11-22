
/*
Authors:1.Lavanya Juvvala
	 2.B V SaiNaveena 
	 3.Rishikesh Jangam
*/

#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include <math.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
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
file_p = fopen(fileName, "w+");

	if(file_p == NULL)
		return RC_FILE_NOT_FOUND;
	else{
	//	fseek(file_p, PAGE_SIZE, SEEK_SET); -> this does not wwork
	// instead create empty page in memory and write it to file

		SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));  //create Empty Page
		if(fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_p) < PAGE_SIZE){  //write Empty page to file
		printf("fwrite wrong \n");
		}else{
			printf("fwrite correct \n");
		}
		//fseek(file_p, PAGE_SIZE, SEEK_SET);
		fclose(file_p);  //always close file
		free(EmptyPage);
		return RC_OK;
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
	file_p = fopen(fileName, "r+");  //open file in read mode

	if(file_p == NULL)
		return RC_FILE_NOT_FOUND;
	else{ 
		//update file handle
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;	
	
	/*	fseek(file_p, 0L, SEEK_END);
		file_pos = ftell(file_p);
		fseek(file_p, 0L, SEEK_SET);
		fHandle.totalNumPages = int(file_pos)/ PAGE_SIZE;
	*/
			//use file stats to get file size and derive number of pages
			struct stat fileStat;
			if(fstat(fileno(file_p),&fileStat) < 0)    
		return RC_ERROR;
			fHandle->totalNumPages = fileStat.st_size/ PAGE_SIZE;
		
			fclose(file_p);
			return RC_OK;
		
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
{	if(file_p!=NULL)
		file_p=NULL;	
	return RC_OK; 
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
	file_p = fopen(fileName, "r"); //open file with the paramter file name
	
	if(file_p == NULL)
		return RC_FILE_NOT_FOUND; 
		
	remove(fileName); //begone File
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
	file_p = fopen(fHandle->fileName, "r"); //open file in r mode
	if(file_p == NULL){
		return RC_FILE_NOT_FOUND; // error check
	}
	if (pageNum > fHandle->totalNumPages || pageNum < 0) {
	memPage = NULL;				//Added by Shouvik
        fclose(file_p);
        return RC_READ_NON_EXISTING_PAGE; 
    	}
	fseek(file_p, (pageNum * PAGE_SIZE), SEEK_SET);
	if(fread(memPage, 1, PAGE_SIZE, file_p) < PAGE_SIZE)
		return RC_ERROR;
    	fHandle->curPagePos = ftell(file_p); //update fhandle
    	fclose(file_p); //always close file_p
	
    	return RC_OK;
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
	file_p = fopen(fHandle->fileName, "r+");	//Open file in read mode	
	if(file_p == NULL)
		return RC_FILE_NOT_FOUND;	//Throw error if filepointer is null
	int i;
	for(i=0;i<PAGE_SIZE; i++){
		char c = fgetc(file_p); // Reading one char from the file.		
		if(feof(file_p)){	//if filepointer pointing to end of file then break.
			break;
		}
		else
			memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_p);// set current file position to curPagePos
	fclose(file_p);	//closing filepointer
	return RC_OK;
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
	if(fHandle->curPagePos <= PAGE_SIZE){	//condition to check if current position of file is on first page.
		printf("File pointer on the first block. no previous block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
		int startpos= (PAGE_SIZE*(curpagenum - 2)); // storing the previous page start position
		file_p=fopen(fHandle->fileName,"r+"); //Open file in read mode
		if(file_p == NULL)
			return RC_FILE_NOT_FOUND;  //Throw error if filepointer is null	
		fseek(file_p,startpos,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading previous block character by character and storing in memPage
			memPage[i] = fgetc(file_p);
		}
		fHandle->curPagePos = ftell(file_p);// set current file position to curPagePos
		fclose(file_p); //closing filepointer
		return RC_OK;				
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
	int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.
	int startpos= (PAGE_SIZE*(curpagenum - 1)); // storing the current page start position
	
	file_p=fopen(fHandle->fileName,"r+"); //Open file in read mode
	if(file_p == NULL)
		return RC_FILE_NOT_FOUND; //Throw error if filepointer is null		
	fseek(file_p,startpos,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE;i++){ //reading current block character by character and storing in memPage
		char c = fgetc(file_p);		
		if(feof(file_p))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_p);// set current file position to curPagePos	
	fclose(file_p); //closing filepointer
	return RC_OK;	
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
	int curpagenum = ceil((float)fHandle->curPagePos/(float)PAGE_SIZE); //Calculating current Page number.	
	if(fHandle->totalNumPages == curpagenum){ //condition to check if current position of file is on last page.
		printf("File pointer on the last block. No next block to read.");
		return RC_READ_NON_EXISTING_PAGE;	
	}	
	else{
		int startpos= (PAGE_SIZE*curpagenum); // storing the next page start position
		
		file_p=fopen(fHandle->fileName,"r+"); //Open file in read mode
		if(file_p == NULL)
			return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
		fseek(file_p,startpos,SEEK_SET);
		int i;
		for(i=0;i<PAGE_SIZE;i++){ //reading next block character by character and storing in memPage
			char c = fgetc(file_p);
			if(feof(file_p))
				break;
			memPage[i] = c;
		}
		fHandle->curPagePos = ftell(file_p);// set current file position to curPagePos
		fclose(file_p); //closing filepointer
		return RC_OK;							
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
	file_p = fopen(fHandle->fileName, "r+"); //Open file in read mode	
	if(file_p == NULL)
		return RC_FILE_NOT_FOUND; //Throw error if filepointer is null
	
	int startpos = (fHandle->totalNumPages - 1) * PAGE_SIZE; // storing the last page start position	
	fseek(file_p,startpos,SEEK_SET);
	int i;
	for(i=0;i<PAGE_SIZE; i++){ //reading last block character by character and storing in memPage
		char c = fgetc(file_p);
		if(feof(file_p))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(file_p);// set current file position to curPagePos
	fclose(file_p); //closing filepointer		
	return RC_OK;
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
	file_p = fopen(fHandle->fileName, "r+");// open file in read+write mode.
	if(file_p == NULL)
		return RC_FILE_NOT_FOUND; // Throw an error file not found.
	long int curPosition = fHandle->curPagePos; //Storing current file position.
	
	appendEmptyBlock(fHandle); //Appending an empty block to make space for the new content.

	fseek(file_p,curPosition,SEEK_SET); //Seek to the current position.
	
	int ctr=0;
	while(fgetc(file_p)!= EOF) //Calculating the total number of character after the point where we need to insert new data.
		ctr++;
	fseek(file_p,curPosition,SEEK_SET);
	//char *string1 = malloc(PAGE_SIZE+1);
	//fread(string1,1,ctr-PAGE_SIZE,file_p); //Storing in string1 the content after the current position.
	
	fseek(file_p,curPosition,SEEK_SET);

	fwrite(memPage,1,strlen(memPage),file_p);//Writing the memPage to our file.
	//fwrite(string1,1,strlen(string1),file_p);//Writing the string1 to our file.
	
	fHandle->curPagePos = ftell(file_p); // set current file position to curPagePos
	//free(string1);	//free string memory
	fclose(file_p); //closing filepointer
	return RC_OK;
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
	SM_PageHandle EmptyPage = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char)); //creating empty page of PAGE_SIZE bytes 
	fseek(file_p, 0, SEEK_END);
	fwrite(EmptyPage,sizeof(char),PAGE_SIZE,file_p); //Writing Empty page to the file.
	free(EmptyPage); //free memory from EmptyPage.
	fHandle->totalNumPages++; //Increasing total number of pages.
	return RC_OK;
}

/*
	Function Name: ensureCapacity
	Input Arguments: int numberOfile_pages, SM_FileHandle *fHandle
	Description:
	1) Find the sppecified file,if it fails return an appropriate error message if not, continue.
	2) Calculate the number of pages that the file has.
	3) If the file's memory is not sufficient, calculate the memory needed so that the file has required capacity.

*/
extern RC ensureCapacity(int numberOfile_pages, SM_FileHandle *fHandle) //increasing the file size if number of pages are less
{	file_p=fopen(fHandle->fileName,"a");
	if(file_p==NULL)
		return RC_FILE_NOT_FOUND;
	while(numberOfile_pages > fHandle->totalNumPages) //If numberOfile_pages is greater than totalNumPages then add emptly pages.
		appendEmptyBlock(fHandle);

	fclose(file_p);
	return RC_OK;
}