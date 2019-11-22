Assignment 3: Record manager

Team members: 
1)Venkata Sai Naveena Bathula      A20451804    <vbathula@hawk.iit.edu>
2)Rishikesh Jangam                        A20448930    <rjangam@hawk.iit.edu>                
3)Hita Venkatesh Prasanna             A20443457    <hbenkateshprasanna@hawk.iit.edu>

INSTRUCTIONS TO RUN THE CODE:

Go to Project root (Assignment3) using Terminal

Type "make clean" to delete old compiled .o files.

Type "make" to compile all project files including "test_assign3_1.c" file

Type "make run" to run "test_assign3_1.c" file.

Type "make test_expr" to compile test file "test_expr.c".

Type "make run_expr" to run "run_expr.c" file.


Brief ideas:

Functions use two Structures named RM_TableMgmtData and RM_ScanMgmtData.

RM_Table_Data : maintain all pointer of Buffer Pool and information of total number of tuples, First free page RM_Scan_Data : maintain pointer of buffer pool page handle and scan information like scan count, condition expression.

Tombstone : Tombstone is an efficient implementation for lazy deletion of tuples. We used one byte character for it. Its the first byte of every tuple.

 '#' :- it represents that slot has a record meaning its a non-empty slot.
 '0' :- (zero) it represents that slot is empty or that a previous value has been deleted; ready for insertion.
Example of Record Data Structure:
| Tombstone | Attribute-1 (INT) | Attribute-2 (STRING) | Attribute-3 (INT) |
| 1 byte | 4 bytes | Length of String (eg : 4 bytes) | 4 bytes |

 Total Record Size : 13 byte
 		
 Non-Empty Record : "#1aaaa3"
 Empty Record     : "0_ _ _ _ _ _"
 Deleted Record   : "03cccc5" 
Page0 in the Page File contains only information, no records. It contains: Number of tuples in the table, First page with empty slots in the file, the Schema of the Table.

We used LFU as the Page replacement Algorithm, Since Page0 will be frequently used we felt its good if it remains in memory //initBufferPool(&tableMgmt->bm, name, size, RS_LFU, NULL);

FUNCTIONS USED:

Table and record manager functions:
initRecordManager Function  -This Function initializes the record manager.
shutdownRecordManager Function-This Function shutdowns the record manager.
createTable Function-This Functions creates a table. It creates the underlying page file and store information about the schema, free-space and so on in the table information                                    pages.
openTable Function-This Function is used to open a table.
closeTable Function-This Function is used to close a table. It causes all outstanding changes to the tables to be written to the page file. 
deleteTable Function-This Function deletes the table.
getNumTuples Function-This Function returns the number of tuples in the table.

Record functions:
insertRecord Function-This Function inserts a new record.This newly inserted record will be assigned a RID and the record parameter passed is updated. 
deleteRecord Function-This Function deletes the record with a certain RID passed as an parameter.
updateRecord Function-This Function updates a existing record with new values.
getRecord Function-This Function retrieve a record with certain RID passed as an parameter.

Scan functions:
startScan Function-This Function initializes the RM_ScanHandle data structure passed as an argument along with it.
next Function-This Function returns the next tuple that fullfills the scan condition,if NULL is passed as an scan condition,then all tuples of the table should is returned.
This function return RC_RM_NO_MORE_TUPLES once the scan is completed and RC_OK otherwise unless an error occurs. 
closeScan Function-This Function indicates to the record manager that all associated resources can be cleaned up.

Schema funtions:
getRecordSize Function-This Function returns the size in bytes of record for a given schema.
createSchema Function-This Function creates a new Schema.
freeSchema Function-This function frees a given schema.

Attribute functions:
createRecord Function-This Function creates a new record for a given schema and it allocates enough memory to the data field to hold the binary representation for all                                     attributes of this record as determined by the schema.
freeRecord Function-This Function frees a specified record.
getAttr Function-This Function gets the attribute values of a record.
setAttr Function-This Function sets the attribute values of a record.
	
ADDITIONAL FUNCTIONALITIES:

We have implemented the following:

Record Manager all function
Record with the Tombstone

