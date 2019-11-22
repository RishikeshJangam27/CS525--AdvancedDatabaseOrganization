#include "buffer_mgr.h"
#include "storage_mgr.h"

#include<stdio.h>
#include<stdlib.h>
#include <math.h>


typedef struct frame
{
	SM_PageHandle data;
	int dirty;
	int fixedCount;
	int numHit;	
	int nReference;
	PageNumber pagenumber;
} BufferManager;

int bSize = 0;
int rear = 0;
int writeCount = 0;
int hit = 0; 
int clockpr = 0;
int lfuPr = 0;


RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData)
{
	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	BufferManager *bf = malloc(sizeof(BufferManager)*numPages);
		
	bSize =numPages;	
	int i;
	for(i=0;i<bSize;i++)
	{
		bf[i].data = NULL;
		bf[i].pagenumber = -1;
		bf[i].dirty = 0;
		bf[i].fixedCount = 0;
		bf[i].numHit = 0;	
		bf[i].nReference = 0;
	}
	bm->mgmtData = bf;
	writeCount = 0;
	clockpr = 0;
	lfuPr = 0;
	return RC_OK;
		
}


RC shutdownBufferPool(BM_BufferPool *const bm)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	//printf("\nshutdownBufferPool");	
	forceFlushPool(bm);	
	int i;	
	for(i=0;i<bSize;i++)
	{
		if(bf[i].fixedCount != 0)
			return RC_PINNED_PAGES_STILL_IN_BUFFER;
	}
	free(bf);
	bm->mgmtData = NULL;
	return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	//printf("\nforceFlushPool");	
	int i;	
	for(i=0;i<bSize;i++)
	{
		if(bf[i].dirty == 1 && bf[i].fixedCount == 0)
		{
			SM_FileHandle filehandle;
			openPageFile (bm->pageFile, &filehandle);
			writeBlock (bf[i].pagenumber, &filehandle, bf[i].data);
			bf[i].dirty = 0;
			writeCount++;
		}
	}	
	return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	int i;	
	for(i=0;i<bSize;i++)
	{
		if(bf[i].pagenumber == page->pageNum)
		{
			bf[i].dirty = 1;
			return RC_OK;		
		}			
	}		
	return RC_ERROR;
}
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
		
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	int i;	
	for(i=0;i<bSize;i++)
	{
		if(bf[i].pagenumber == page->pageNum)
		{
			bf[i].fixedCount--;
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
			writeBlock (bf[i].pagenumber, &filehandle, bf[i].data);
			bf[i].dirty = 0;
			writeCount++;
		}
	}	
	return RC_OK;
}



void fifo(BM_BufferPool *const bm, BufferManager *newFrame) //Fifo function 
{
	//printf("\nInside fifo function");	
	BufferManager *bf=(BufferManager *) bm->mgmtData;
	
	int i,front = rear%bSize;	
	for(i=0;i<bSize;i++)
	{
		if(bf[front].fixedCount == 0)
		{
			if(bf[front].dirty == 1)
			{
				SM_FileHandle filehandle;
				openPageFile (bm->pageFile, &filehandle);
				writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);
				writeCount++;
			}
			
			bf[front].data = newFrame->data;
			bf[front].pagenumber = newFrame->pagenumber;
			bf[front].dirty = newFrame->dirty;
			bf[front].fixedCount = newFrame->fixedCount;
			break;
		}
		else
		{
			front++;
			if(front%bSize == 0)
				front=0;
		}
	}
}

void LFU(BM_BufferPool *const bm, BufferManager *newFrame){

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
		
		
	if(bf[front].dirty == 1)
	{
		SM_FileHandle filehandle;
		openPageFile (bm->pageFile, &filehandle);
		writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);
		writeCount++;
	}
			
	bf[front].data = newFrame->data;
	bf[front].pagenumber = newFrame->pagenumber;
	bf[front].dirty = newFrame->dirty;
	bf[front].fixedCount = newFrame->fixedCount;
	lfuPr=front+1;
	
	}


void clock(BM_BufferPool *const bm,BufferManager *newFrame)
{
	//printf("\nInside clock function");	
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
		}
	}
}

void lru(BM_BufferPool *const bm, BufferManager *newFrame)
{
	//printf("\nInside lru function");	
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
	for(i=front+1;i<bSize;i++)
	{
		if(bf[i].numHit < minimum)
		{
			front = i;
			minimum = bf[i].numHit;
		}
	}
	if(bf[front].dirty == 1)
	{
		SM_FileHandle filehandle;
		openPageFile (bm->pageFile, &filehandle);
		writeBlock (bf[front].pagenumber, &filehandle, bf[front].data);
		writeCount++;
	}
	
	bf[front].data = newFrame->data;
	bf[front].pagenumber = newFrame->pagenumber;
	bf[front].dirty = newFrame->dirty;
	bf[front].fixedCount = newFrame->fixedCount;
	bf[front].numHit = newFrame->numHit;
}



RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	BufferManager *bf = (BufferManager *)bm->mgmtData;
	
	if(bf[0].pagenumber == -1)
	{
		//printf("\nINSIDE ds[0]->pagenum == -1\n");
		
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
		
		/*	
		printf("\nPinPage function buffer");
		int i;
		for(i=0;i<bufferSize; i++)
		{
			printf("\nPagenum: %d fixedcnt: %d dirty: %d",ds[i].pagenum,ds[i].fixedcnt,ds[i].dirtybit);	
		}*/
		
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
				if(bf[i].pagenumber == pageNum)  //if Page already in memory
				{
					bf[i].fixedCount++;
					check = 1;
					hit++;
					if(bm->strategy == RS_LRU)	
						bf[i].numHit = hit;
					else if(bm->strategy == RS_CLOCK)		//if bm-> strategy is RS_CLOCK storing the USED bit in hitnum.
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

			switch(bm->strategy)
			{			
				case RS_FIFO:	//printf("\n Inside FIFO switch.");
						fifo(bm,temp);
						break;
				case RS_LRU:	//printf("\n Inside LRU switch.");
						lru(bm,temp);  						
						break;
				case RS_CLOCK:	//printf("\n Inside CLOCK switch");
						clock(bm,temp);						
						break;
  				case RS_LFU:	//printf("\n Inside LFU switch");
  						LFU(bm,temp);
						break;
  				case RS_LRU_K:	printf("\n Inside LRU_K switch");
						break;
				default:
								printf("\nAlgorithm Not Implemented\n");
						break;
			}//end of switch
						
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
{
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
{
	bool *p = malloc(sizeof(bool)*bSize);
	BufferManager *bf= (BufferManager *)bm->mgmtData;
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
{
	int *p = malloc(sizeof(int)*bSize);
	BufferManager *bf= (BufferManager *)bm->mgmtData;
	int i;	
	for(i=0;i<bSize;i++)
		p[i] = bf[i].fixedCount;
		
	return p;
}

int getNumReadIO (BM_BufferPool *const bm)
{
	return (rear+1);
	
}
int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
}
