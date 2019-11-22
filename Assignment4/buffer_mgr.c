#include "buffer_mgr.h"
#include "storage_mgr.h"
#include<stdio.h>
#include<stdlib.h>
#include <math.h>

//This is a structure representing a single frame in memory (within the buffer pool)

typedef struct frame
{
	SM_PageHandle data; // Data contained in the page
	int dirty;          //Indicated whether page contents have been changed
	int fixedCount;     // Denotes the number of users at an instance of time
	int numHit;	        // Indicates the number of page hits
	int nReference;     // Denotes the page that i minimally used for the LRU algorithm
	PageNumber pagenumber;      // Page identifier
} BufferManager;

//The buffer manager data structure represents the various fields that are a part of the buffer manager

int bSize = 0;   // Initial size of the buffer pool
int rear = 0;    // Count of pages that is read
int writeCount = 0; // Count of number of writes to disk
int hit = 0;      // variable that gets incremented upon the addition of a page frame
int clockpr = 0; // A pointer for the page that is recently added
int lfuPr = 0;   // Stores the page frame that is least frequently used



/*
   The initialization function will create a buffer pool, when the number of pages is specified in terms of number of page frames.
   A particular replacement strategy can be used.
*/


RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy; //Replacement algorithms like FIFO,LRU,LFU, CLOCK can be used.

	// Used to reserve memory space which is a ;product of the number of pages and the size of each page
	BufferManager *bf = malloc(sizeof(BufferManager)*numPages);

	bSize =numPages; //The buffer size will be the number of pages present in the buffer pool or memory
	int i;
	for(i=0;i<bSize;i++) // all the pages pertaining to the buffer pool is initialized
	{
		bf[i].data = NULL;
		bf[i].pagenumber = -1;
		bf[i].dirty = 0;
		bf[i].fixedCount = 0;
		bf[i].numHit = 0;
		bf[i].nReference = 0;
	}
	bm->mgmtData = bf;  // The contents (management data is stored
	writeCount = 0;     // Initially no writes are performed
	clockpr = 0;
	lfuPr = 0;
	return RC_OK;       // Returns an OK message once the buffer manager has been successfully initialized

}

// This function is used to close the buffer pool. The resources that had been allocated will be released and made available for reuse.

RC shutdownBufferPool(BM_BufferPool *const bm)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	//printf("\nshutdownBufferPool");
	// The dirty pages must be written back
	forceFlushPool(bm);	 // This will forcefully flush the contents of the buffer manager
	int i;
	// The buffer manager will be iterated through from the first element
	for(i=0;i<bSize;i++)
	{
		if(bf[i].fixedCount != 0) // If true, then this indicated that some content that had been modified by the user has not been written on to the disk.
			return RC_PINNED_PAGES_STILL_IN_BUFFER; // This is used to denote the presence of pinned pages within the buffer manager
	}
	free(bf);   // Deallocate the memory in order to free the resourced that were consumed
	bm->mgmtData = NULL;
	return RC_OK;    // Returns OK on successful completion
}

// The pages having the value of fixcount as zero, considered to be dirty will be written on to the disk.
RC forceFlushPool(BM_BufferPool *const bm)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;

	int i;
	for(i=0;i<bSize;i++) // To store the modified pages on to the disk
	{
		if(bf[i].dirty == 1 && bf[i].fixedCount == 0)   // A dirty bit denoted that the page has been modified
		{
			SM_FileHandle filehandle; // To begin with the file handling operations
			openPageFile (bm->pageFile, &filehandle); // The file will first have to be opened
			writeBlock (bf[i].pagenumber, &filehandle, bf[i].data); // A block of data will be written back on to the disk.
			bf[i].dirty = 0; // Page will be marked as not-dirty
			writeCount++; // increments the value of the writes made by the Buffermanager
		}
	}
	return RC_OK;   // Successfully performed operation
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{  // This method is used to mark a certain page after it has been modified to indicate the modification has been made
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	int i;
	for(i=0;i<bSize;i++) // All the pages will be passed through one after the other
	{
		if(bf[i].pagenumber == page->pageNum)  // Once the required page is found
		{                                     // mark the page as dirty
			bf[i].dirty = 1;  // Indicated that the particular page has been modified
			return RC_OK;
		}
	}
	return RC_ERROR;
}

// This function will be used to unpin a certain page
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{

	BufferManager *bf = (BufferManager *)bm->mgmtData;
	int i;
	for(i=0;i<bSize;i++)
	{
		if(bf[i].pagenumber == page->pageNum)
		{
			bf[i].fixedCount--; // If the current page is selected to be unpinned then the value of fixed count will be decremented
			break;
		}
	}
	/*
	printf("\nAfter Unpinning: ");
	for(i=0;i<bufferSize;i++)
		printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);
	*/
	return RC_OK;
}

// This is a method that is used to write the data contained in the page that has been changed back on to the disk
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	//printf("\nforcePage");
	int i;
	for(i=0;i<bSize;i++)
	{
		if(bf[i].pagenumber == page->pageNum)
		{
			SM_FileHandle filehandle;
			openPageFile (bm->pageFile, &filehandle);
			writeBlock (bf[i].pagenumber, &filehandle, bf[i].data);    //Write to keep the page on the disk updated
			bf[i].dirty = 0;       // Set the dirty bit to zero as the modified page has been written on to the disk
			writeCount++;          // Increment the write count in order to maintain the updated count for number of writes performed
		}
	}
	return RC_OK;
}



void fifo(BM_BufferPool *const bm, BufferManager *newFrame) // Function that defined the first in first out page replacement strategy
{
	//printf("\nInside fifo function");
	BufferManager *bf=(BufferManager *) bm->mgmtData;

	int i,front = rear%bSize; //    Initializing the front and rear of the queue
	for(i=0;i<bSize;i++)    //Iterating throughout all the pages
	{
		if(bf[front].fixedCount == 0)
		{
			if(bf[front].dirty == 1) // Write a page on to the disk only if has been modified
			{
				SM_FileHandle filehandle;
				openPageFile (bm->pageFile, &filehandle);
				writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);
				writeCount++; // The count will be incremented upon each write performed by the buffer manager
			}
			bf[front].data = newFrame->data;			// The page frame's content will be set to store the new content

			bf[front].pagenumber = newFrame->pagenumber;
			bf[front].dirty = newFrame->dirty;
			bf[front].fixedCount = newFrame->fixedCount;
			break;
		}
		else
		{
			front++;// The front will be incremented if the current page is being used by one of the users
			if(front%bSize == 0)
				front=0;    // The first page has been replaced
		}
	}
}

void LFU(BM_BufferPool *const bm, BufferManager *newFrame){ // Function definition for the least frequently used algorithm

	BufferManager *bf=(BufferManager *) bm->mgmtData;

	int i,
	front = lfuPr;
	//printf("front : %d \n", front);
	int minimum, count = 0;

	for(i=0;i<bSize;i++)
	{
		if(bf[front].fixedCount == 0)
		{
			front = (front + i)%bSize;
		//	printf("front : %d \n", front);
			minimum = bf[front].nReference;
		//	printf("reference count: %d \n", min);
			break;
		}
	}
	i = (front+1)%bSize;
	for(count = 0;count < bSize;count++)
	{
		if(bf[i].nReference < minimum)
		{
			front = i;
			minimum = bf[i].nReference;
		}
		i = (i+1)%bSize;

	}
// The goal of this replacement algorithm is to find the page that has been least used or referenced

	if(bf[front].dirty == 1)
	{
		SM_FileHandle filehandle;
		openPageFile (bm->pageFile, &filehandle);
		writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);
		writeCount++;
	}
//Finds the least frequently used page
	bf[front].data = newFrame->data;
	bf[front].pagenumber = newFrame->pagenumber;
	bf[front].dirty = newFrame->dirty;
	bf[front].fixedCount = newFrame->fixedCount;
	lfuPr=front+1;
//Replaces the least frequently used page by new page
	}


void clock(BM_BufferPool *const bm,BufferManager *newFrame) // The clock function will be defined
{
	BufferManager *bf=(BufferManager *) bm->mgmtData;
	for(;;)
	{
		if(clockpr%bSize == 0)
			clockpr = 0;
		if(bf[clockpr].numHit == 0)
		{
			if(bf[clockpr].dirty == 1)
			{
				SM_FileHandle filehandle;
				openPageFile (bm->pageFile, &filehandle);
				writeBlock (bf[clockpr].pagenumber, &filehandle, bf[clockpr].data);
				writeCount++;
			}
			// The content of the page frame will be set to point to the new content
			bf[clockpr].data = newFrame->data;
			bf[clockpr].pagenumber = newFrame->pagenumber;
			bf[clockpr].dirty = newFrame->dirty;
			bf[clockpr].fixedCount = newFrame->fixedCount;
			bf[clockpr].numHit = newFrame->numHit;
			clockpr++;
			break;
		}
		else
		{
			bf[clockpr].numHit = 0;
			clockpr++;
			// The reason for incrementing the clock pointer is to location of the next page frame
		}
	}
}

void lru(BM_BufferPool *const bm, BufferManager *newFrame)
{
     // Function for the least recently used page replacement strategy
	BufferManager *bf=(BufferManager *) bm->mgmtData;

	int i,front, minimum;
	for(i=0;i<bSize;i++)
	{
		if(bf[i].fixedCount == 0)
		{
			front= i;
			minimum = bf[i].numHit;
			break;
		}
	}
	// Iterates through the entire buffer manager to find the page frame that has been least recently used
	for(i=front+1;i<bSize;i++)
	{
		if(bf[i].numHit < minimum)
		{
			front = i;
			minimum = bf[i].numHit;
		}
	}
	if(bf[front].dirty == 1)   // Finds the page that has been modified
	{
		SM_FileHandle filehandle;
		openPageFile (bm->pageFile, &filehandle);
		writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);   // Writes to keep the buffer manager updated
		writeCount++;
	}
//The new page frame's data is added to keep the buffer manager updated
	bf[front].data = newFrame->data;
	bf[front].pagenumber = newFrame->pagenumber;
	bf[front].dirty = newFrame->dirty;
	bf[front].fixedCount = newFrame->fixedCount;
	bf[front].numHit = newFrame->numHit;
}



RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
	    const PageNumber pageNum)
{

      // A method that is used to pin a certain page with page number pageNum i.e. adds the page with page number  to the buffer pool.
	BufferManager *bf = (BufferManager *)bm->mgmtData;

	if(bf[0].pagenumber == -1)
	{
		//printf("\nINSIDE ds[0]->pagenum == -1\n");
		// A page replacement strategy will be used when the buffer pool is full
		SM_FileHandle filehandle;
		openPageFile (bm->pageFile, &filehandle);
		bf[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&filehandle);
		readBlock(pageNum, &filehandle, bf[0].data);
		bf[0].pagenumber = pageNum;
		bf[0].fixedCount++;
		rear = 0;
		hit = 0;
		bf[0].numHit = hit;
		bf[0].nReference = 0;
		page->pageNum = pageNum;
		page->data = bf[0].data;


		return RC_OK;
	}
	else
	{
		//printf("\nINSIDE front != NULL\n");
		int i,check = 0;

		for(i=0;i<bSize;i++)
		{
			if(bf[i].pagenumber != -1)
			{
				if(bf[i].pagenumber == pageNum)  //his checks if Page already in memory
				{
					bf[i].fixedCount++;
					check = 1;
					hit++;
					// This replacement strategy uses the value of hit to determine which page was recently used
					if(bm->strategy == RS_LRU)
						bf[i].numHit = hit;
					else if(bm->strategy == RS_CLOCK)		//This is used to check whether this is the last page that was added
						bf[i].numHit = 1;
					else if(bm->strategy == RS_LFU)
						{
						bf[i].nReference++;
						//printf("Page %d referenced again \n", pageNum);
						//rear = rear + 2;
						//printf(" rear : %d \n", rear);
						}


					page->pageNum = pageNum;
					page->data = bf[i].data;

					clockpr++;

					break;
				}
			}
			else	//Condition when the buffer has space to add a page
			{
				SM_FileHandle filehandle;
				openPageFile (bm->pageFile, &filehandle);
				bf[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &filehandle, bf[i].data);
				bf[i].pagenumber = pageNum;
				bf[i].fixedCount = 1;
				bf[i].nReference = 0;
				rear++;
				hit++;
				if(bm->strategy == RS_LRU)
					bf[i].numHit = hit;
				else if(bm->strategy == RS_CLOCK)		//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
					bf[i].numHit = 1;


				page->pageNum = pageNum;
				page->data = bf[i].data;

				check = 1;
				break;
			}
		}//end of for

		if(check == 0)//Condition when the buffer is full and we need to use a replacement strategy.
		{
			BufferManager *temp = (BufferManager *)malloc(sizeof(BufferManager));

			SM_FileHandle filehandle;
			openPageFile (bm->pageFile, &filehandle);
			temp->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &filehandle, temp->data);
			temp->pagenumber = pageNum;
			temp->dirty = 0;
			temp->fixedCount = 1;
			temp->nReference = 0;
			rear++;
			hit++;
			//printf("HIT : %d \n", hit); //test by Rakesh
			if(bm->strategy == RS_LRU )
				temp->numHit = hit;
			else if(bm->strategy == RS_CLOCK)			//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
				temp->numHit = 1;

			page->pageNum = pageNum;
			page->data = temp->data;
  // This is used to call an appropriate function to handle using the replacement strategy
			switch(bm->strategy)  // Uses a specific replacement strategy for memory management
			{
				case RS_FIFO:	//printf("\n Inside FIFO switch.");
						fifo(bm,temp);// This makes use of the First-In-First-Out replacement strategy
						break;
				case RS_LRU:	//printf("\n Inside LRU switch.");
						lru(bm,temp);  	// This makes use of the least-Recently-Used page replacement strategy
						break;
				case RS_CLOCK:	//printf("\n Inside CLOCK switch");
						clock(bm,temp);	// This uses the clock algorithm
						break;
  				case RS_LFU:	//printf("\n Inside LFU switch");
  						LFU(bm,temp);
						break;
  				case RS_LRU_K:	printf("\n Inside LRU_K switch");
						break;
				default:
								printf("\nAlgorithm Not Implemented\n"); // In case there is no need for any replacement strategy to be applied
						break;
			}

		}//end of if(check = 0)

		/*
		printf("\nPinPage function buffer");
		for(i=0;i<bufferSize;i++)
			printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);
		*/
		return RC_OK;
	}//end of else
}

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm)
{  // Means for an array of page numbers to be returned
	PageNumber *p = malloc(sizeof(PageNumber)*bSize);
	BufferManager *bf= (BufferManager *)bm->mgmtData;
	int i;
	for(i=0;i<bSize;i++)
		p[i] = bf[i].pagenumber;

	/*printf("\ngetFrameContents: ");
	for(i=0;i<bufferSize;i++)
		printf("%i ",x[i]);
	*/
	return p;
}
bool *getDirtyFlags (BM_BufferPool *const bm)
{  // This is used to return an array of Bool's
	bool *p = malloc(sizeof(bool)*bSize);
	BufferManager *bf= (BufferManager *)bm->mgmtData; // Each element would represent the dirty bit of a certain page
	int i;
	for(i=0;i<bSize;i++)
	{
		if(bf[i].dirty == 1)
			p[i]= true;
		else
			p[i]=false;
	}
	return p;
}
int *getFixCounts (BM_BufferPool *const bm)
{ // An array of integers are returned
	int *p = malloc(sizeof(int)*bSize);
	BufferManager *bf= (BufferManager *)bm->mgmtData;
	int i;
	for(i=0;i<bSize;i++)
		p[i] = bf[i].fixedCount;

	return p;
}

int getNumReadIO (BM_BufferPool *const bm) // Indicates the number of pages that have bee read since the last initialization
{
	return (rear+1); // In order to start with the rear index of zero

}
int getNumWriteIO (BM_BufferPool *const bm)  // Indicates the number of pages that have bee written since the last initialization
{
	return writeCount;
}
