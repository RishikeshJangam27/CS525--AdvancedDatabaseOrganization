#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dt.h"
#include "dberror.h"
#define PAGE_SIZE 8192
#define TABLE_BUFFER_LENGTH 10
#define HEADER_LENGTH 40
#define SIZE_INT sizeof(int)


#include "dberror.h"
#include "tables.h"

typedef struct Tree {
  int length;
  int str;
  int *Obj;        
} Tree;

typedef struct btree {
  
  Tree *val;
  Tree *Child_Page;
  Tree *L_Page;
  Tree *L_Spot;
  int cap;
  int Leaf;
  int PageNum;

  //pointers to btrees
  struct btree *Parent;
  struct btree **Child;
  struct btree *Right;
  struct btree *Left;
} btree;

//structures for accessing btrees
typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
  int cap;
  int TotalNumEntries;
  int TotalNumNodes;
  int level;
  int Root_Loc;
  int Next;
  btree *root;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;

typedef struct ScanInfor {
  btree *presNode;
  int position;
} ScanInfor;



btree *createBtreeNode(int cap, int Leaf, int PageNum);

// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);

#endif // BTREE_MGR_H
