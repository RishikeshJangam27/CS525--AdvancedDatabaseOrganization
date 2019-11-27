/*
Authors:1. Rishikesh Jangam
	 2. B V SaiNaveena 
	 3.Hita Prasanna
*/
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <math.h>



typedef struct RM_Table_Data
{
	// Buffer Manager PageHandle
	BM_PageHandle pHandle;
	// Buffer Manager Buffer Pool
    BM_BufferPool bm;
	// Number of tuples
    int numberofTuples;	
	// first free page which contain empty slots
    int freePage;		
} RM_Table_Data;

//Scanning tuples in the Table
typedef struct RM_Scan_Data
{
	// current row to be scanned
	RID rid;  
	// number of tuples scanned
    int count;
	// condition  to be checked
    Expr *c; 
    // Buffer Manager PageHandle
    BM_PageHandle pHandle;
    
} RM_Scan_Data;
int count = 0;
int size = 35;
RM_Table_Data *table_Mgmt_Data;
// table and manager
extern RC initRecordManager (void *mgmtData)
{
	// initialize Storage Manager inside Record Manager
	initStorageManager();	
	return RC_OK;
}
extern RC shutdownRecordManager ()
{
	
	return RC_OK;
}

/**************************************************************************************
> function name: createTable
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
// Creating a Table with filename named as name
extern RC createTable (char *name, Schema *schema)
{
    SM_FileHandle fHandle;
    // Allocating the memory for table management data 	
    table_Mgmt_Data = (RM_Table_Data*) malloc( sizeof(RM_Table_Data) ); 
    initBufferPool(&table_Mgmt_Data->bm, name, size, RS_LFU, NULL);

    char data[PAGE_SIZE];
	char *pHandle = data;
	 
	RC rc;
	int i;
	memset(pHandle, 0, PAGE_SIZE);
	// No. of tuples
	*(int*)pHandle = 0; 
	//incrementing char pointer
    pHandle+= sizeof(int); 
	// First free page is 1 as page 0 
	*(int*)pHandle = 1; 
	//incrementing char pointer
    pHandle += sizeof(int); 
	//setting number of attributes
	*(int*)pHandle = schema->numAttr; 
    pHandle += sizeof(int); 
	// setting keySize of attributes
    *(int*)pHandle = schema->keySize; 
    pHandle += sizeof(int);
	
	for(i=0; i<schema->numAttr; i++)
    {
       // setting an Attribute Name
       strncpy(pHandle, schema->attrNames[i], 10);	 
       pHandle += 10;
		// Setting the Data Types of an Attribute
       *(int*)pHandle = (int)schema->dataTypes[i];	
       pHandle += sizeof(int);
		// Setting the Length of an Attribute
       *(int*)pHandle = (int) schema->typeLength[i];	
       pHandle += sizeof(int);
    }
	// Creating file for table using Storage Manager Function	
	rc = createPageFile(name);		
	if(rc != RC_OK)
        return rc;
	// opening Created File	
	rc = openPageFile(name, &fHandle);	
	if(rc != RC_OK)
        return rc;
	// Writing Schema To 0th page of file	
	rc=writeBlock(0, &fHandle, data);	
	if(rc != RC_OK)
        return rc;
	// Closing File	
	rc=closePageFile(&fHandle);			
	if(rc != RC_OK)
        return rc;
    return RC_OK;
}

/**************************************************************************************
> function name: openTable
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

extern RC openTable (RM_TableData *rel, char *name)
{
  
    SM_PageHandle pHandle;    
    int numAttrs, i;
	// Setting mgmtData
    rel->mgmtData = table_Mgmt_Data;	
	// Setting mgmtData
    rel->name = strdup(name); 		  
    // pins the 0th page
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, (PageNumber)0); 
	// setting char to pointer to 0th page
	pHandle = (char*) table_Mgmt_Data->pHandle.data;	 
	// retrieving the total number of tuples
	table_Mgmt_Data->numberofTuples= *(int*)pHandle; 	
	//printf("Num of Tuples: %d \n", table_Mgmt_Data->numberofTuples );
    pHandle+= sizeof(int);
    
	// retrieving free page
	table_Mgmt_Data->freePage= *(int*)pHandle;	 
	//printf("First free page: %d \n", table_Mgmt_Data->freePage );
    pHandle+= sizeof(int);
	// retrieving number of Attributes
    numAttrs = *(int*)pHandle;		  
	pHandle+= sizeof(int);
 	
 	// Setting all values to Schema
    Schema *schema;
    schema= (Schema*) malloc(sizeof(Schema));
  
    schema->numAttr= numAttrs;
    schema->attrNames= (char**) malloc( sizeof(char*)*numAttrs);
    schema->dataTypes= (DataType*) malloc( sizeof(DataType)*numAttrs);
    schema->typeLength= (int*) malloc( sizeof(int)*numAttrs);

    for(i=0; i < numAttrs; i++)
		//10 is maximum field length
       schema->attrNames[i]= (char*) malloc(10); 
      
   for(i=0; i < schema->numAttr; i++)
    {
       strncpy(schema->attrNames[i], pHandle, 10);
       pHandle += 10;
	   
	   schema->dataTypes[i]= *(int*)pHandle;
       pHandle += sizeof(int);

       schema->typeLength[i]= *(int*)pHandle;
       pHandle+=sizeof(int);
    }
	// setting schema object to rel object
	rel->schema = schema;  	
    // Unpinning after reading
    unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle);
    forcePage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle);
    return RC_OK;    
   
}   

/**************************************************************************************
> function name: closeTable
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

extern RC closeTable (RM_TableData *rel)
{
	RM_Table_Data *table_Mgmt_Data;
	//setting rel->mgmtData value to table_Mgmt_Data
	table_Mgmt_Data = rel->mgmtData;	
	//Shutdown Buffer Pool
	shutdownBufferPool(&table_Mgmt_Data->bm);	 
	rel->mgmtData = NULL;	
	return RC_OK;
}
/**************************************************************************************
> function name: deleteTable
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
// Deleting the Table
extern RC deleteTable (char *name)
{
	//free(table_Mgmt_Data);
	// removing  file
	destroyPageFile(name);	 
	return RC_OK;
}

/**************************************************************************************
> function name: getNumTuples
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
extern int getNumTuples (RM_TableData *rel)
{
		
		RM_Table_Data *tdata;	 
    	tdata = rel->mgmtData;
	
	return tdata->numberofTuples;
}
/**************************************************************************************
> function name: getFreeSlot
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
int getFreeSlot(char *data, int record_Size)
{
	
    int i;
    int total_Slots = floor(PAGE_SIZE/record_Size); 

    for (i = 0; i < total_Slots; i++)
    {
    	
        if (data[i * record_Size] != '#')
		{
            return i;
            
            }
    }
    return -1;
}

/**************************************************************************************
> function name: insertRecord
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
// handling records
extern RC insertRecord (RM_TableData *rel, Record *record){
	
	RM_Table_Data *table_Mgmt_Data;
	table_Mgmt_Data = rel->mgmtData;	
	
	// setting "rid" from current Record
	RID *rid = &record->id;	 
	
	char *data;
	char * slotAdr;
	// record size of a certain Record
	int recordSize = getRecordSize(rel->schema);	 
	// setting First Free page to current page
	rid->page = table_Mgmt_Data->freePage; 
	// pinning first free page 
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, rid->page);	
	
	
	// setting char pointer to current page data
	data = table_Mgmt_Data->pHandle.data;
	// retrieving 1st free slot of current pinned page
	rid->slot = getFreeSlot(data, recordSize);	

	while(rid->slot == -1)
	{
	// if page doesn't have free slot then unpin that page	
	unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle);	
	// incrementing page number
	rid->page++;	
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, rid->page);
	data = table_Mgmt_Data->pHandle.data;
	rid->slot = getFreeSlot(data, recordSize);
	}
	
	
	slotAdr = data;
	
	//write record
	markDirty(&table_Mgmt_Data->bm,&table_Mgmt_Data->pHandle);
	
	slotAdr += rid->slot * recordSize;
	*slotAdr = '#';
	slotAdr++;
	
	memcpy(slotAdr, record->data + 1, recordSize - 1);
	
	
	unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle);
	
	table_Mgmt_Data->numberofTuples++;
		
	// to be removed
	
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, 0);
	data = table_Mgmt_Data->pHandle.data;
	
	return RC_OK;
}

/**************************************************************************************
> function name: deleteRecord
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

// Deleting Record From Table having with certain RID

extern RC deleteRecord (RM_TableData *rel, RID id){

	RM_Table_Data *table_Mgmt_Data = rel->mgmtData;
	// pinning page which have record that needs to be deleted
    pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, id.page); 
	//updating number of tuples
	table_Mgmt_Data->numberofTuples--; 
	//updating first free page
	table_Mgmt_Data->freePage = id.page; 
	// setting char pointer to pinned page
	char * data = table_Mgmt_Data->pHandle.data; 
	// retrieving number of tuples
	*(int*)data =  table_Mgmt_Data->numberofTuples; 
	// gets Record Size
	int recordSize = getRecordSize(rel->schema);
	// setting the pointer to a slot of record
	data += id.slot * recordSize; 
	// setting tombstone '0' for deleted record
	*data = '0'; 
	// marking page as Dirty	
	markDirty(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle); 
	// unpin the page
	unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle); 

	return RC_OK;
}
/**************************************************************************************
> function name: updateRecord
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/
// Updating Record of Table
extern RC updateRecord (RM_TableData *rel, Record *record){
		
	RM_Table_Data *table_Mgmt_Data = rel->mgmtData;
	// pinning the record which we need to Update
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, record->id.page); 
	char * data;
	RID id = record->id;
	
	data = table_Mgmt_Data->pHandle.data;
	// gets size of Record
	int recordSize = getRecordSize(rel->schema); 
	// setting pointer to slot
	data += id.slot * recordSize; 
	// setting tombstone as '#'
	*data = '#'; 
    // incrementing data pointer by 1
	data++; 
	//update the record with new record
	memcpy(data,record->data + 1, recordSize - 1 ); 
	// marking page as dirty
	markDirty(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle); 
	// unpin the page
	unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle); 
	
	return RC_OK;	
}
/**************************************************************************************
> function name: getRecord
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

// gets a given record from Table
extern RC getRecord (RM_TableData *rel, RID id, Record *record){
	RM_Table_Data *table_Mgmt_Data = rel->mgmtData;
		
	// pinning the record which we need to get
	pinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle, id.page); 
	// gets size of Record	
	int recordsize = getRecordSize(rel->schema); 
	char * slotAdr = table_Mgmt_Data->pHandle.data;
	slotAdr+=id.slot*recordsize;
	if(*slotAdr != '#')
		// return code for not matching record
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID; 
	else{
		// setting pointer to record data
		char *tar = record->data; 
		*tar='0';
		tar++;
		// get record data
		memcpy(tar,slotAdr+1,recordsize-1);
		// setting Record ID		
		record->id = id; 
        
		char *slotAdr1 = record->data;
		
		
        // incrementing pointer to next attribute
		char *temp1 = slotAdr1+1; 
        
		
		temp1 += sizeof(int);
		char * string = (char *)malloc(5);
		strncpy(string,temp1 , 4);
		string[4] = '\0';
		printf("2nd ATTR: %s \t", string);
		temp1 += 4;
		printf("3RD ATTR: %d \n", *(int*)temp1);
		free(string);
		
	}
	// Unpin the page after getting record
	unpinPage(&table_Mgmt_Data->bm, &table_Mgmt_Data->pHandle); 
	return RC_OK;
}

/**************************************************************************************
> function name: startScan
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

// scanning is Start this method
int fl;
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
	
	
		closeTable(rel);
   		openTable(rel, "test_table_r");
 
    	RM_Scan_Data *scanMgmt;
		//allocating memory for scan Data
    	scanMgmt = (RM_Scan_Data*) malloc( sizeof(RM_Scan_Data) ); 
    	
    	scan->mgmtData = scanMgmt;
    	// starting scan from 1st page
    	scanMgmt->rid.page= 1;     
		// starting scan from 1st slot
    	scanMgmt->rid.slot= 0;      
		// count of number of scan - n
    	scanMgmt->count= 0; 
		// set the condition of scan
    	scanMgmt->c = cond; 
    	RM_Table_Data *tdata;
    	 
    	tdata = rel->mgmtData;
		// setting the number of tuples
    	tdata->numberofTuples = 10;    
    	scan->rel= rel;
    	fl = 0;
	return RC_OK;
}
/**************************************************************************************
> function name: next
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

extern RC next (RM_ScanHandle *scan, Record *record)
{
	
	
	RM_Scan_Data *scanMgmt; 
	scanMgmt = (RM_Scan_Data*) scan->mgmtData;
    RM_Table_Data *tdata;
    tdata = (RM_Table_Data*) scan->rel->mgmtData;	
    
     Value *result = (Value *) malloc(sizeof(Value));
   
  	 static char *data;
   
     int recordSize = getRecordSize(scan->rel->schema);
     int totalSlots = floor(PAGE_SIZE/recordSize);
    	if (tdata->numberofTuples == 0)
    	    return RC_RM_NO_MORE_TUPLES;
  
	//all tuples are scanned
	while(scanMgmt->count <= tdata->numberofTuples ){   
		if (scanMgmt->count <= 0)
		{
		    scanMgmt->rid.page = 1;
		    scanMgmt->rid.slot = 0;
		    
		    pinPage(&tdata->bm, &scanMgmt->pHandle, scanMgmt->rid.page);
		    data = scanMgmt->pHandle.data;
		}
		else
		{
			scanMgmt->rid.slot++;
			if(scanMgmt->rid.slot >= totalSlots){
			scanMgmt->rid.slot = 0;
			scanMgmt->rid.page++;
			}
			
			pinPage(&tdata->bm, &scanMgmt->pHandle, scanMgmt->rid.page);
		    data = scanMgmt->pHandle.data;
		}
        
		data += scanMgmt->rid.slot * recordSize;
		record->id.page=scanMgmt->rid.page;
		record->id.slot=scanMgmt->rid.slot;
		scanMgmt->count++;
		
		char *tar = record->data;
		*tar='0';
		tar++;
		
		memcpy(tar,data+1,recordSize-1);
	      
		if (scanMgmt->c != NULL){
			evalExpr(record, (scan->rel)->schema, scanMgmt->c, &result); 
			}
		
		//v.BoolV is true 
		if(result->v.boolV == TRUE){  
		    unpinPage(&tdata->bm, &scanMgmt->pHandle);
		    fl = 1;
			return RC_OK;  
		}
	}
    
	    unpinPage(&tdata->bm, &scanMgmt->pHandle);
	    scanMgmt->rid.page = 1;
	    scanMgmt->rid.slot = 0;
	    scanMgmt->count = 0;
            
	return RC_RM_NO_MORE_TUPLES;
       
}
/**************************************************************************************
> function name: closeScan
> Author: Rishikesh Jangam
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
/// Closing Scan
extern RC closeScan (RM_ScanHandle *scan)
{
	RM_Scan_Data *scanMgmt= (RM_Scan_Data*) scan->mgmtData;
	RM_Table_Data *table_Mgmt_Data= (RM_Table_Data*) scan->rel->mgmtData;
	
	//if scan is incomplete
	if(scanMgmt->count > 0)
	{
	// unpin the page
	unpinPage(&table_Mgmt_Data->bm, &scanMgmt->pHandle); 
	 // reset scanMgmt to 1st page
	 scanMgmt->rid.page= 1; 
	 // reset scanMgmt to 1st slot
     scanMgmt->rid.slot= 0;
	 // reset count to 0
     scanMgmt->count = 0; 
	}
        // Free mgmtData   
    	scan->mgmtData= NULL;
    	free(scan->mgmtData);  
	return RC_OK;
}
/**************************************************************************************
> function name: getRecordSize
> Author: Rishikesh Jangam
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

//give the Record Size
extern int getRecordSize (Schema *schema){
	//setting offset to zero
	int offset = 0, i = 0; 
	//total number of attribute
	for(i = 0; i < schema->numAttr; i++)
	{
		// check the data types of attributes
		switch(schema->dataTypes[i])
		{  
			 case DT_STRING:
				//increment offset according to its typeLength
				offset += schema->typeLength[i];  
				break;
			  case DT_INT:
				//increment offset to size of INTEGER
				offset += sizeof(int); 
				break;
			  case DT_FLOAT:
				//increment offset to size of FLOAT
				offset += sizeof(float); 
				break;
			  case DT_BOOL:
				// increment offset to size of BOOLEAN
				offset += sizeof(bool); 
				break;
		}
	}
	// plus 1 for end of string
	offset += 1; 
	return offset;
}

/**************************************************************************************
> function name: createSchema
> Author: Rishikesh Jangam
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

// Creating Schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	// allocating memory to schema
	Schema *result = (Schema *)malloc(sizeof(Schema)); 
	
	// setting Key size
	result->keySize = keySize;  
	// setting key attribute
	result->keyAttrs = keys;
	// setting Number of Attribute
	result->numAttr = numAttr; 
	// setting Attribute Names
	result->attrNames = attrNames; 
	// setting  Attribute's data types
	result->dataTypes =dataTypes;
	// setting Attribute's type length
	result->typeLength = typeLength; 
 
	
	return result; 
}

/**************************************************************************************
> function name: freeSchema
> Author: Rishikesh Jangam
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

// Free schema
extern RC freeSchema (Schema *schema)
{
	free(schema); 
	return RC_OK;
}
/**************************************************************************************
> function name: createRecord
> Author: Rishikesh Jangam
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

// Creating a Blank record
extern RC createRecord (Record **record, Schema *schema)
{
	// allocating memory to empty record
    Record *tRecord = (Record*) malloc( sizeof(Record) );
	// gets size of Record
	int recordSize = getRecordSize(schema); 
    // Allocating memory for data
	tRecord->data= (char*) malloc(recordSize);
	// setting char pointer to data
    char * temp = tRecord->data; 
	// setting tombstone '0'
	*temp = '0'; 
	
    temp += sizeof(char);
	// setting null value to record
	*temp = '\0'; 
	// page number is not fixed for empty record
    tRecord->id.page= -1; 
	// slot number is not fixed for empty record
    tRecord->id.slot= -1; 
	// setting tRecord to Record
    *record = tRecord; 
    return RC_OK;
}

/**************************************************************************************
> function name: attrOffset
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
// return position of an attribute
RC attrOffset (Schema *schema, int attrNum, int *result)
{
  int offset = 1;
  int atrPos = 0;
  //number of attribute
  for(atrPos = 0; atrPos < attrNum; atrPos++)  
    // check dataTypes of attributes
	switch (schema->dataTypes[atrPos]) 
      {
      case DT_STRING:
	//increment offset according to its typeLength
	offset += schema->typeLength[atrPos]; 
	break;
      case DT_INT:
	//increment offset to size of INTEGER
	offset += sizeof(int); 
	break;
      case DT_FLOAT:
	//increment offset to size of FLOAT
	offset += sizeof(float); 
	break;
      case DT_BOOL:
	//increment offset to size of BOOLEAN
	offset += sizeof(bool); 
	break;
      }
  
  *result = offset;
  return RC_OK;
}

/**************************************************************************************
> function name: freeRecord
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

//after using free the record
extern RC freeRecord (Record *record)
{
	// Free memory of record
	free(record); 
	return RC_OK;
}
/**************************************************************************************
> function name: getAttr
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

// get Attribute
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{

	int offset = 0; 
	//offset to the given attribute number
	attrOffset(schema, attrNum, &offset); 
	// allocating memory to value object
    Value *tvalue = (Value*) malloc(sizeof(Value)); 

	char *string = record->data;
	string += offset;
//	printf("\n attrNum : %d \n\n", attrNum);
	if(attrNum == 1){
	schema->dataTypes[attrNum] = 1;
	}
	
	 switch(schema->dataTypes[attrNum])
    {
	// gets attribute value from record of type Integer
    case DT_INT: 
      {
		int val = 0;
		memcpy(&val,string, sizeof(int));
		tvalue->v.intV = val;
		tvalue->dt = DT_INT;
      }
      break;
	// gets attribute value from record of type String
    case DT_STRING: 
      {

    tvalue->dt = DT_STRING;
	
	int len = schema->typeLength[attrNum];
	tvalue->v.stringV = (char *) malloc(len + 1);
	strncpy(tvalue->v.stringV, string, len);
	tvalue->v.stringV[len] = '\0';
	
      }
      break;
	// gets attribute value from record of type Float
    case DT_FLOAT: 
      {
      tvalue->dt = DT_FLOAT;
	  float val;
	  memcpy(&val,string, sizeof(float));
	  tvalue->v.floatV = val;
      }
      break;
	// gets attribute value from record of type Boolean
    case DT_BOOL: 
      {
	  tvalue->dt = DT_BOOL;
	  bool val;
	  memcpy(&val,string, sizeof(bool));
	  tvalue->v.boolV = val;
      }
      break;
    default:
      	printf("NO SERIALIZER FOR DATATYPE \n\n\n\n");
    }
    
 	*value = tvalue;
	return RC_OK;
}
/**************************************************************************************
> function name: setAttr
> Author: B.V. Naveena
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

// Setting the Attribute value
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
		int offset = 0;
		// retrieving the attribute offset
		attrOffset(schema, attrNum, &offset); 
		char *data = record->data;
		data += offset;
		
		switch(schema->dataTypes[attrNum])
		{
		// setting the attribute value of type Integer
		case DT_INT: 
			*(int *)data = value->v.intV;	  
			data += sizeof(int);
		  	break;
		// setting the attribute value of type String
		case DT_STRING: 
		  {
			char *buffer;
			// length of string
			int len = schema->typeLength[attrNum];
			// allocating the memory to buffer
			buffer = (char *) malloc(len + 1); 
			// copy string to buffer
			strncpy(buffer, value->v.stringV, len); 
			buffer[len] = '\0';
			// copy data to buffer
			strncpy(data, buffer, len); 
			// free memory of buffer
			free(buffer); 
			data += schema->typeLength[attrNum];
		  }
		  break;
		// setting the attribute value of type Float
		case DT_FLOAT: 
		  {
			 // sets value of attribute
			*(float *)data = value->v.floatV;	
			// incrementing the data pointer
			data += sizeof(float); 
		  }
		  break;
		// setting the attribute value of type Boolean
		case DT_BOOL: 
		  {
			// copy the boolean value
			*(bool *)data = value->v.boolV;	
			data += sizeof(bool);
		  }
		  break;
		default:
		  printf("NO SERIALIZER FOR DATATYPE");
		}
				
	return RC_OK;
}

