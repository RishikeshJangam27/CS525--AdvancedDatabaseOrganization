ADO assignment1
______________________________________________________________________
Group members information:


1. Lavanya Juvvala               CWID:A20442790  ljuvvala@hawk.iit.edu
2. Venkata Sai Naveena Bathula   CWID:A20451804	vbathula@hawk.iit.edu
3. Rishikesh Jangam              CWID:A20448930  rjangam@hawk.iit.edu
______________________________________________________________________
List of Files:

1. README.md
2. dberror.c
3. dberror.h
4. storage_mgr.c
5. storage_mgr.h
6. test_assign1_1.c
7. test_assign1_2.c
8. test_helper.h
9. Makefile1
10. Makefile2

______________________________________________________________________
Instructions for installation:

1. Go to the path(ado_storage_manager_fall2019) in the Ubuntu-VMware terminal.
2. Type 'ls' and look for the Makefile.
3. Type 'make -f Makefile1' to compile all the files in it.
4. Type 'make -f Makefile2' to compile all the files in it.
5. Type 'ls' and find 'test1' and 'test2' files.
6. Type './test1' to run test cases from the 'test_assign1_1.c' file.
7. Type './test2' to run the test cases from the 'test_assign1_2.c' file.

______________________________________________________________________
Description of the functions:

1) File functions
  i)createPageFile    : This method is used for  creating a file with the specified 'fileName' and initialized with '\0' bytes.
  ii)openPageFile     : fopen() function is used to open the file. If the file is found it will return RC_OK and when the file is not found it will return RC_FILE_NOT_FOUND.
  iii)closePageFile   : fclose() function is used to close the file. This will return an Error message when the file pointer is pointed to NULL Value.
  iv)destroyPageFile  : This function will delete the files if it exists else an error is thrown.

2)Read functions
  i)readBlock          : This method will read the page file and store into its memory.
  ii)getBlockPos       : This method will return the current page position from the file handle's curPagePos.
  iii)readFirstBlock   : This method checks if the file exists and then move the file descriptor to first page and read content to memPage.
  iv)readLastBlock     : This method checks if the file already exists and move the file descriptor to last page and read content to memPage.
  v)readPreviousBlock  : This method is used to fetch a block from file that is one block before the current page position.
  vi)readCurrentBlock  : This method is used to fetch the block from file at current page position.
  vii)readNextBlock    : This method is used to fetch the block one after current page position, by providing pageNum argument as current page position +1.

3)Write functions
  i)writeBlock         : This method sets the file pointer from current position to the required page and writes the content into that page.
  ii)writeCurrentBlock : This method is similar to writeBlock() function except that currenyt psoition of the file pointer is used to write the data.
  iii)appendEmptyBlock : This method is used to add one empty block at the end of the file. The total number of pages is incremented by 1.
  iv)ensureCapacity    : This method is used to check if the file's memory is less than the number of pages. If yes update the capacity.
 
TEST CASES 

> This program verifies the test cases mentioned in file "test_assign1_1.c" which execute correctly.
> We added additional test cases in the file " test_assign1_2.c "which also executes correctly. 
