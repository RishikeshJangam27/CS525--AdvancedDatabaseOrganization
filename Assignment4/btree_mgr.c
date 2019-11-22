/*
Authors:
	1. Rishikesh Jangam - rjangam@hawk.iit.edu
	2. B V SaiNaveena - vbathula@hawk.iit.edu
	3.Hita Prasanna - hbenkateshprasanna@hawk.iit.edu
*/

#include <unistd.h>
#include <math.h>
#include <stdarg.h>

#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "string.h"

//additional functions 
/**************************************************************************************
> function name: Tree_Find
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
int Tree_Find(Tree *list, int element, int *suit) 
{
  int init = 0;
  int end;
  end = list->str - 1;
  //checks if there is an empty array
  if (end < 0) 
  {                              
    (*suit) = init;
    return -1;
  }
  int position;
  while(true) 
  {
	//finding exactly the median position
    position = (init + end) / 2;              
    if(element == list->Obj[position]) 
	{
      (*suit) = position;
      return position;
    }
    if(init >= end) 
	{
      if (element > list->Obj[init]) 
	  {
        init++;
      }
      (*suit) = init;
      return -1;
    }
    if(element < list->Obj[position]) 
	{
      end = position - 1;
    }
    else 
	{
	//setting initial position as next position
      init = position + 1;            
    }
  }
}

/**************************************************************************************
> function name: Tree_initialize
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
Tree *Tree_initialize(int length) 
{
  Tree *list;
  list = new(Tree);
  list->Obj = newIntArr(length);
  list->str = 0;
  list->length = length;
  return list;
}
/**************************************************************************************
> function name: Tree_Delete
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

//function to delete elements
void Tree_Delete(Tree *list) 
{
  free(list->Obj);
  free(list);
}
/**************************************************************************************
> function name: Tree_Place_Position
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
int Tree_Place_Position(Tree *list, int element, int index) 
{
  if (list->length > list->str && index <= list->str) 
  {
    if (index != list->str) 
	{
      int i = list->str;
      while(i > index) 
	  {
        list->Obj[i] = list->Obj[i - 1];
        i = i - 1;
      }
    }
    list->Obj[index] = element;
    list->str++;
    return index;
  }
  return -1;
}

/**************************************************************************************
> function name: Tree_Place
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

int Tree_Place(Tree *list, int element) 
{
	//checking for space management
  int suit = -1;                                  
  if (list->length > list->str) 
  {
    int index;
    index = Tree_Find(list, element, &suit);
    suit = Tree_Place_Position(list, element, suit);
  }
  return suit;
}
/**************************************************************************************
> function name: Tree_Remove_Position
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

void Tree_Remove_Position(Tree *list, int index, int count) 
{
  list->str = list->str - count;
  int i = index;
  while(i < list->str) 
  {
    list->Obj[i] = list->Obj[i + count];
    i = i + 1;
  }
}

/**************************************************************************************
> function name: MakeEmpty
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/
void MakeEmpty(int num, ...) 
{
  int i = 0;
  va_list var;
  va_start(var, num);
  while(i < num) 
  {
    free(va_arg(var, void *));
    i++;
  }
  va_end(var);
}

btree *Build_Btree(int cap, int Leaf, int PageNum) 
{
  btree *node = new(btree);
  node->Parent = NULL;
  node->Right = NULL;
  node->Left = NULL;
  node->PageNum = PageNum;
  node->cap = cap;
  node->Leaf = Leaf;
  node->val = Tree_initialize(cap);
  
  int leaf_node;
  if(Leaf) 
  {
  	leaf_node = 0;
  }
  switch(leaf_node)
  {
  	case 0:
        		node->L_Page = Tree_initialize(cap);
          	node->L_Spot = Tree_initialize(cap);
    default:
          	node->Child_Page = Tree_initialize(cap + 1);
          	node->Child = newArray(btree *, cap + 1);
  }
  return node;
}
/**************************************************************************************
> function name: Remove_Node
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

void Remove_Node(btree *node) 
{
  Tree_Delete(node->val);
  int check_leaf = 0;
  if (node->Leaf)
  {
  	check_leaf = 1;
  }
  switch(check_leaf) 
  {
  	case 1:
          	Tree_Delete(node->L_Page);
          	Tree_Delete(node->L_Spot);

  	default:
          	Tree_Delete(node->Child_Page);
          	free(node->Child);
  }
  free(node);
}
/**************************************************************************************
> function name: Display_Btree
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/
RC Display_Btree(btree *node, char *output) 
{
  if (node == NULL) 
  {
    sprintf(output + strlen(output),"Node = NULL\n");
    return RC_GENERAL_ERROR;
  }
  sprintf(output + strlen(output), "(%d)[", node->PageNum);

  int i, check_leaf = 0;
  if(node->Leaf)
  {
  	check_leaf = 1;
  }
  switch(check_leaf)
  {
  	case 1:
      	    while(i < node->val->str) 
			{
      	      sprintf(output + strlen(output),"%d", node->L_Page->Obj[i]);
      	      sprintf(output + strlen(output),".%d,", node->L_Spot->Obj[i]);
      	      sprintf(output + strlen(output),"%d", node->val->Obj[i]);
              int check = i < node->val->str - 1;
      	      if(check)
			  {
      	        sprintf(output + strlen(output),",");
      	      }
      	      i++;
      	    }
  	default:
      	    while(i < node->val->str) 
			{
      	      sprintf(output + strlen(output),"%d,", node->Child_Page->Obj[i]);
      	      sprintf(output + strlen(output),"%d,", node->val->Obj[i]);
      	      i++;
      	    }
      	    sprintf(output + strlen(output),"%d", node->Child_Page->Obj[i]);
        }
        sprintf(output + strlen(output), "]\n");
        return RC_OK;
}

/**************************************************************************************
> function name: Scan_Btree
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

RC Scan_Btree(btree **node, BTreeHandle *binary_tree, int PageNum) 
{
  RC error;
  BM_PageHandle *page_handle;
  page_handle = new(BM_PageHandle);
  if (RC_OK!=(error = pinPage(binary_tree->mgmtData, page_handle, PageNum))) 
  {
    free(page_handle);
    return error;
  }

  int Leaf;
  char *ptr;
  ptr = page_handle->data;
  memcpy(&Leaf, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  int str;
  memcpy(&str, ptr, SIZE_INT);
  ptr = page_handle->data + HEADER_LENGTH;
  btree *Node;
  Node = Build_Btree(binary_tree->cap, Leaf, PageNum);
  int value, i = 0, check_leaf = 0;
  int C_Node_Page;
  int RID_Page, RID_Spot;
  if (!Leaf) 
  {
  	check_leaf = 1;
  }
  switch (check_leaf) 
  {
  	case 1:
      	    while(i < str) 
			{
      	      memcpy(&C_Node_Page, ptr, SIZE_INT);
      	      ptr = ptr + SIZE_INT;
      	      memcpy(&value, ptr, SIZE_INT);
      	      ptr = ptr + SIZE_INT;
              Tree_Place_Position(Node->Child_Page, C_Node_Page, i);
      	      Tree_Place_Position(Node->val, value, i);
      	      i++;
      	    }
      	    memcpy(&C_Node_Page, ptr, SIZE_INT);
      	    Tree_Place_Position(Node->Child_Page, C_Node_Page, i);

  	default:
        		i = 0;
      	    while(i < str) 
			{
      	      memcpy(&RID_Page, ptr, SIZE_INT);
      	      ptr = ptr + SIZE_INT;
      	      memcpy(&RID_Spot, ptr, SIZE_INT);
      	      ptr = ptr + SIZE_INT;
      	      memcpy(&value, ptr, SIZE_INT);
      	      ptr = ptr + SIZE_INT;
      	      Tree_Place_Position(Node->val, value, i);
      	      Tree_Place_Position(Node->L_Page, RID_Page, i);
      	      Tree_Place_Position(Node->L_Spot, RID_Spot, i);
      	      i++;
      	    }
  }

  error = unpinPage(binary_tree->mgmtData, page_handle);
  free(page_handle);
  *node = Node;
  return error;
}

/**************************************************************************************
> function name: Write_Node
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

RC Write_Node(btree *node, BTreeHandle *binary_tree) 
{
  RC error;
  BM_PageHandle *page_handle;
  page_handle = new(BM_PageHandle);
  if (RC_OK!=(error = pinPage(binary_tree->mgmtData, page_handle, node->PageNum))) 
  {
    free(page_handle);
    return error;
  }

  char *ptr;
  ptr = page_handle->data;
  memcpy(ptr, &node->Leaf, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(ptr, &node->val->str, SIZE_INT);
  ptr = page_handle->data + HEADER_LENGTH;

  int i = 0;
  int check_leaf = 0;
  if (!node->Leaf){
  	check_leaf = 1;
  }
  switch(check_leaf)
  {
  	case 1:
    		    while(i < node->val->str) 
				{
    		      memcpy(ptr, &node->Child_Page->Obj[i], SIZE_INT);
    		      ptr = ptr + SIZE_INT;
    		      memcpy(ptr, &node->val->Obj[i], SIZE_INT);
    		      ptr = ptr + SIZE_INT;
    		      i++;
    		    }
    		    memcpy(ptr, &node->Child_Page->Obj[i], SIZE_INT);

  	default:
    		    while(i < node->val->str) 
				{
    		      memcpy(ptr, &node->L_Page->Obj[i], SIZE_INT);
    		      ptr = ptr + SIZE_INT;
    		      memcpy(ptr, &node->L_Spot->Obj[i], SIZE_INT);
    		      ptr = ptr + SIZE_INT;
    		      memcpy(ptr, &node->val->Obj[i], SIZE_INT);
    		      ptr = ptr + SIZE_INT;
    		      i++;
    		    }
  }
  error = markDirty(binary_tree->mgmtData, page_handle);
  error = unpinPage(binary_tree->mgmtData, page_handle);
  forceFlushPool(binary_tree->mgmtData);
  free(page_handle);
  return error;
}
/**************************************************************************************
> function name: Load_Btree_Node
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

RC Load_Btree_Node(BTreeHandle *binary_tree, btree *root, btree **left_node, int level) 
{
  btree *Left;
  Left = left_node[level];
  RC error;
  int i = 0;
  if(!root->Leaf) 
  {
    while(i < root->Child_Page->str) 
	{
      if ((error = Scan_Btree(&root->Child[i], binary_tree, root->Child_Page->Obj[i]))) 
	  {
        return error;
      }
      if (Left != NULL) 
	  {
        Left->Right = root->Child[i];
      }
      root->Child[i]->Left = Left;
      Left = root->Child[i];
      root->Child[i]->Parent = root;
      left_node[level] = Left;
      if ((error = Load_Btree_Node(binary_tree, root->Child[i], left_node, level + 1))) 
	  {
        return error;
      }
      i++;
    }
  }
  return RC_OK;
}

btree *Locate_Node(BTreeHandle *binary_tree, int key) 
{
  btree *cur;
  cur = binary_tree->root;
  int t, place;
  while(cur != NULL && !cur->Leaf) 
  {
    t = Tree_Find(cur->val, key, &place);
    if (t >= 0) 
	{
      place = place + 1;
    }
    cur = cur->Child[place];
  }
  return cur;
}

/**************************************************************************************
> function name: Load_Btree
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

RC Load_Btree(BTreeHandle *binary_tree) 
{
  RC error;
  binary_tree->root = NULL;
  int i = 0;
  int check = binary_tree->level;
  if (check) 
  {
    if ((error = Scan_Btree(&binary_tree->root, binary_tree, binary_tree->Root_Loc))) 
	{
      return error;
    }
    btree **left_node;
    left_node = newArray(btree *, binary_tree->level);
    while(i < binary_tree->level) 
	{
      left_node[i] = NULL;
      i++;
    }
    error = Load_Btree_Node(binary_tree, binary_tree->root, left_node, 0);
    free(left_node);
    return error;
  }
  return RC_OK;
}

/**************************************************************************************
> function name: Output_Btree
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
RC Output_Btree(BTreeHandle *binary_tree) 
{
  RC error;
  BM_BufferPool *bm = binary_tree->mgmtData;
  BM_PageHandle *page_handle; 
  page_handle = new(BM_PageHandle);
  if (RC_OK != (error = pinPage(bm, page_handle, 0))) 
  {
    MakeEmpty(1, page_handle);
    return error;
  }
  error = markDirty(bm, page_handle);
  error = unpinPage(bm, page_handle);
  forceFlushPool(bm);
  MakeEmpty(1, page_handle);
  return error;
}

/**************************************************************************************
> function name: Move_Parent
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
RC Move_Parent(BTreeHandle *binary_tree, btree *Left, btree *Right, int key) 
{
  btree *Parent;
  Parent = Left->Parent;
  int index, i;
  if(Parent == NULL) 
  {
    Parent = Build_Btree(binary_tree->cap, 0, binary_tree->Next);
    Tree_Place_Position(Parent->Child_Page, Left->PageNum, 0);
    Parent->Child[0] = Left;
    binary_tree->Next++;
    binary_tree->Root_Loc = Parent->PageNum;
    binary_tree->TotalNumNodes++;
    binary_tree->level++;
    binary_tree->root = Parent;
    Output_Btree(binary_tree);
  }
  Right->Parent = Parent;
  Left->Parent = Parent;
  index = Tree_Place(Parent->val, key);
  int check_index = 0;
  btree * overflow = NULL;
  if (index >= 0) 
  {
    check_index = 1;
  }
  switch(check_index) 
  {
    case 1:
          index = index + 1;
          Tree_Place_Position(Parent->Child_Page, Right->PageNum, index);
          int i = Parent->val->str;
          while(i > index) {
            Parent->Child[i] = Parent->Child[i - 1];
            i--;
          }
          Parent->Child[index] = Right;
          return Write_Node(Parent, binary_tree);

    default:
            overflow = Build_Btree(binary_tree->cap + 1, 0, -1);
            overflow->val->str = Parent->val->str;
            overflow->Child_Page->str = Parent->Child_Page->str;
            memcpy(overflow->val->Obj, Parent->val->Obj, SIZE_INT * Parent->val->str);
            memcpy(overflow->Child_Page->Obj, Parent->Child_Page->Obj, SIZE_INT * Parent->Child_Page->str);
            memcpy(overflow->Child, Parent->Child, sizeof(btree *) * Parent->Child_Page->str);
            index = Tree_Place(overflow->val, key);
            Tree_Place_Position(overflow->Child_Page, Right->PageNum, index + 1);
            i = Parent->Child_Page->str;
            while(i > index + 1) 
			{
              overflow->Child[i] = overflow->Child[i - 1];
              i--;
            }
            overflow->Child[index + 1] = Right;

            int left_sib = overflow->val->str / 2;
            int right_sib = overflow->val->str - left_sib;
            btree *r = Build_Btree(binary_tree->cap, 0, binary_tree->Next);
            binary_tree->Next++;
            binary_tree->TotalNumNodes++;
            Parent->val->str = left_sib;
            Parent->Child_Page->str = left_sib + 1;
            int left_size = Parent->Child_Page->str;
            memcpy(Parent->val->Obj, overflow->val->Obj, SIZE_INT * left_sib);
            memcpy(Parent->Child_Page->Obj, overflow->Child_Page->Obj, SIZE_INT * left_size);
            memcpy(Parent->Child, overflow->Child, sizeof(btree *) * left_size);

            r->val->str = right_sib;
            r->Child_Page->str = overflow->Child_Page->str - left_size;
            int right_size = r->Child_Page->str;
            memcpy(r->val->Obj, overflow->val->Obj + left_sib, SIZE_INT * right_sib);
            memcpy(r->Child_Page->Obj, overflow->Child_Page->Obj + left_size, SIZE_INT * right_size);
            memcpy(r->Child, overflow->Child + left_size, sizeof(btree *) * right_size);

            Remove_Node(overflow);

            key = r->val->Obj[0];
            Tree_Remove_Position(r->val, 0, 1);

            r->Right = Parent->Right;
            int check = r->Right != NULL;
            if (check) 
			{
              r->Right->Left = r;
            }
            Parent->Right = r;
            r->Left = Parent;

            Write_Node(Parent, binary_tree);
            Write_Node(r, binary_tree);
            Output_Btree(binary_tree);
            return Move_Parent(binary_tree, Parent, r, key);
          }
}

/**************************************************************************************
> function name: Remove_Nodes
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

void Remove_Nodes(btree *root) 
{
  if (root == NULL) 
  {
    return;
  }
  btree *leaf_node;
  leaf_node = root;
  for( ;!leaf_node->Leaf; )
  {
    leaf_node = leaf_node->Child[0];
  }
  btree *Parent;
  Parent = leaf_node->Parent;
  btree *next_node;
  while (true) 
  {
    for( ;leaf_node != NULL; )
	{
      next_node = leaf_node->Right;
      Remove_Node(leaf_node);
      leaf_node = next_node;
    }
    if (Parent == NULL) 
	{
      break;
    }
    leaf_node = Parent;
    Parent = leaf_node->Parent;
  }
}

/**************************************************************************************
> function name: initIndexManager
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

//Initialize the Index Manager
RC initIndexManager (void *mgmtData) 
{
  return RC_OK;
}

/**************************************************************************************
> function name: shutdownIndexManager
> Author: Hita Prasanna
> Email: hbenkateshprasanna@hawk.iit.edu
**************************************************************************************/

//shut down the index manager
RC shutdownIndexManager () 
{
  return RC_OK;
}

/**************************************************************************************
> function name: createBtree
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
//creating a b+tree of order n
RC createBtree(char *idxId, DataType keyType, int n)
{
	//checking for tree order greater than n
  if (n > (PAGE_SIZE - HEADER_LENGTH) / (3 * SIZE_INT)) 
  {       
    return RC_IM_N_TO_LAGE;
  }
  RC cor;
  if(RC_OK!= (cor = createPageFile (idxId)))
  {
    return cor;
  }

  SM_FileHandle *file_handle = new(SM_FileHandle);                
  if (RC_OK != (cor = openPageFile(idxId, file_handle))) 
  {
    free(file_handle);
    return cor;
  }

  char *start = newCleanArray(char, PAGE_SIZE);
  char *ptr = start;
  memcpy(ptr, &n, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(ptr, &keyType, SIZE_INT);
  ptr = ptr + SIZE_INT;
  int Root_Loc = 0;
  memcpy(ptr, &Root_Loc, SIZE_INT);
  ptr = ptr + SIZE_INT;
  int TotalNumNodes = 0;
  memcpy(ptr, &TotalNumNodes, SIZE_INT);
  ptr = ptr + SIZE_INT;
  int TotalNumEntries = 0;
  memcpy(ptr, &TotalNumEntries, SIZE_INT);
  ptr = ptr + SIZE_INT;
  int level = 0;
  memcpy(ptr, &level, SIZE_INT);
  int Next = 1;
  ptr = ptr + SIZE_INT;
  memcpy(ptr, &Next, SIZE_INT);

  if (RC_OK != (cor = writeBlock(0, file_handle, start))) 
  {
    free(file_handle);
    free(start);
    return cor;
  }
  free(start);
  cor = closePageFile(file_handle);          
  free(file_handle);
  return cor;
}

/**************************************************************************************
> function name: openBtree
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/
//opening an existing b+tree
RC openBtree (BTreeHandle **binary_tree, char *idxId)
{
  BTreeHandle *bin;
  bin = new(BTreeHandle);
  RC error;
  BM_BufferPool *bm;
  bm = new(BM_BufferPool);
  if ((error = initBufferPool(bm, idxId, TABLE_BUFFER_LENGTH, RS_LRU, NULL))) 
  {
    MakeEmpty(2, bm, bin);
    return error;
  }
  BM_PageHandle *page_handle;
  page_handle = new(BM_PageHandle);                               
  if (RC_OK != (error = pinPage(bm, page_handle, 0))) 
  {
    MakeEmpty(3, bm, page_handle, bin);
    return error;
  }
  char *ptr;
  ptr = page_handle->data;
  bin->idxId = idxId;
  bin->mgmtData = bm;

  memcpy(&bin->cap, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->keyType, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->Root_Loc, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->TotalNumNodes, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->TotalNumEntries, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->level, ptr, SIZE_INT);
  ptr = ptr + SIZE_INT;
  memcpy(&bin->Next, ptr, SIZE_INT);

  if ((error = unpinPage(bm, page_handle)) != RC_OK) 
  {
    MakeEmpty(1, page_handle);
    closeBtree(bin);
    return error;
  }
  MakeEmpty(1, page_handle);
  if ((error = Load_Btree(bin))) 
  {
    closeBtree(bin);
    return error;
  }
  *binary_tree = bin;
  return RC_OK;
}
/**************************************************************************************
> function name: closeBtree
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

//closing the b+tree
RC closeBtree (BTreeHandle *binary_tree)
{
  shutdownBufferPool(binary_tree->mgmtData);                  
  Remove_Nodes(binary_tree->root);
  MakeEmpty(2, binary_tree->mgmtData, binary_tree);
  return RC_OK;                                           
}

//removing the b+tree
RC deleteBtree (char *idxId) 
{
  if(access(idxId, F_OK) == -1) 
  {
    return RC_FILE_NOT_FOUND;
  }
  int re = unlink(idxId);
  if (re == -1) 
  {
    return RC_FS_ERROR;
  }
  return RC_OK;                               
}
/**************************************************************************************
> function name: getNumNodes
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/


//gets the number of nodes in the b+tree
RC getNumNodes (BTreeHandle *binary_tree, int *output) 
{
  *output = binary_tree->TotalNumNodes;
  return RC_OK;
}

/**************************************************************************************
> function name: getNumEntries
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

//gets the number of entries in the b+tree
RC getNumEntries (BTreeHandle *binary_tree, int *output) 
{
  *output = binary_tree->TotalNumEntries;
  return RC_OK;
}

/**************************************************************************************
> function name: getKeyType
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/


//returns the datatype if the keys in the b+tree
RC getKeyType (BTreeHandle *binary_tree, DataType *output) 
{
  return RC_OK;
}

/**************************************************************************************
> function name: findKey
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
//finds a specified key in the b+tree
RC findKey (BTreeHandle *binary_tree, Value *key, RID *output) 
{
  int index, place;
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);
  index = Tree_Find(leaf_node->val, key->v.intV, &place);
  if (index < 0) 
  {
    return RC_IM_KEY_NOT_FOUND;
  }
  output->page = leaf_node->L_Page->Obj[index];
  output->slot = leaf_node->L_Spot->Obj[index];
  return RC_OK;
}
/**************************************************************************************
> function name: insertKey
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/
//placing a new entry in the b+tree
RC insertKey (BTreeHandle *binary_tree, Value *key, RID rid) 
{
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);                            
  if (leaf_node == NULL) 
  {
    leaf_node = Build_Btree(binary_tree->cap, 1, binary_tree->Next);
    binary_tree->Next++;
    binary_tree->Root_Loc = leaf_node->PageNum;
    binary_tree->TotalNumNodes++;
    binary_tree->level++;
    binary_tree->root = leaf_node;
    Output_Btree(binary_tree);
  }
  int index, place;
  index = Tree_Find(leaf_node->val, key->v.intV, &place);
  if (index >= 0) 
  {
    return RC_IM_KEY_ALREADY_EXISTS;
  }
  index = Tree_Place(leaf_node->val, key->v.intV);
  if (index >= 0) 
  {
    Tree_Place_Position(leaf_node->L_Page, rid.page, index);
    Tree_Place_Position(leaf_node->L_Spot, rid.slot, index);
  }
  else 
  {
    btree * overflow = Build_Btree(binary_tree->cap + 1, 1, -1);
    memcpy(overflow->val->Obj, leaf_node->val->Obj, SIZE_INT * leaf_node->val->str);
    overflow->val->str = leaf_node->val->str;
    memcpy(overflow->L_Page->Obj, leaf_node->L_Page->Obj, SIZE_INT * leaf_node->L_Page->str);
    overflow->L_Page->str = leaf_node->L_Page->str;
    memcpy(overflow->L_Spot->Obj, leaf_node->L_Spot->Obj, SIZE_INT * leaf_node->L_Spot->str);
    overflow->L_Spot->str = leaf_node->L_Spot->str;
    index = Tree_Place(overflow->val, key->v.intV);
    Tree_Place_Position(overflow->L_Page, rid.page, index);
    Tree_Place_Position(overflow->L_Spot, rid.slot, index);

    int left_sib = ceil((float) overflow->val->str / 2);
    int right_sib = overflow->val->str - left_sib;
    btree *rightleaf = Build_Btree(binary_tree->cap, 1, binary_tree->Next);
    binary_tree->Next++;
    binary_tree->TotalNumNodes++;
    leaf_node->val->str = leaf_node->L_Page->str = leaf_node->L_Spot->str = left_sib;
    memcpy(leaf_node->val->Obj, overflow->val->Obj, SIZE_INT * left_sib);
    memcpy(leaf_node->L_Page->Obj, overflow->L_Page->Obj, SIZE_INT * left_sib);
    memcpy(leaf_node->L_Spot->Obj, overflow->L_Spot->Obj, SIZE_INT * left_sib);

    rightleaf->val->str = rightleaf->L_Page->str = rightleaf->L_Spot->str = right_sib;
    memcpy(rightleaf->val->Obj, overflow->val->Obj + left_sib, SIZE_INT * right_sib);
    memcpy(rightleaf->L_Page->Obj, overflow->L_Page->Obj + left_sib, SIZE_INT * right_sib);
    memcpy(rightleaf->L_Spot->Obj, overflow->L_Spot->Obj + left_sib, SIZE_INT * right_sib);

    Remove_Node(overflow);

    rightleaf->Right = leaf_node->Right;
    int check = rightleaf->Right != NULL;
    if (check) 
	{
      rightleaf->Right->Left = rightleaf;
    }
    leaf_node->Right = rightleaf;
    rightleaf->Left = leaf_node;
    Write_Node(rightleaf, binary_tree);
    Write_Node(leaf_node, binary_tree);
    Move_Parent(binary_tree, leaf_node, rightleaf, rightleaf->val->Obj[0]);
  }
  binary_tree->TotalNumEntries++;
  Output_Btree(binary_tree);
  return RC_OK;
}

/**************************************************************************************
> function name: deleteKey
> Author: Rishikesh Jangam 
> Email: rjangam@hawk.iit.edu
**************************************************************************************/

//deletes the entry of the specified key
RC deleteKey (BTreeHandle *binary_tree, Value *key) 
{
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);                
  int check = leaf_node != NULL;
  if (check) 
  {
    int index, _unuse;
    index = Tree_Find(leaf_node->val, key->v.intV, &_unuse);
    if (index >= 0) 
	{
      Tree_Remove_Position(leaf_node->val, index, 1);
      Tree_Remove_Position(leaf_node->L_Page, index, 1);
      Tree_Remove_Position(leaf_node->L_Spot, index, 1);
      binary_tree->TotalNumEntries--;
      Write_Node(leaf_node, binary_tree);
      Output_Btree(binary_tree);
    }
  }
  return RC_OK;
}
/**************************************************************************************
> function name: openTreeScan
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

//Scan the entries in the b+tree
RC openTreeScan (BTreeHandle *binary_tree, BT_ScanHandle **handle)
{
  BT_ScanHandle *scan_handle;
  scan_handle = new(BT_ScanHandle);                         
  ScanInfor *data;
  data = new(ScanInfor);
  scan_handle->tree = binary_tree;
  data->presNode = binary_tree->root;
  for( ;!data->presNode->Leaf; ) 
  {
    data->presNode = data->presNode->Child[0];
  }
  data->position = 0;
  scan_handle->mgmtData = data;
  *handle = scan_handle;
  return RC_OK;
}

/**************************************************************************************
> function name: nextEntry
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

//traverse the entries in the b+trees
RC nextEntry (BT_ScanHandle *handle, RID *output)
{
  ScanInfor *data;
  data = handle->mgmtData;
  int check1 = data->position >= data->presNode->L_Page->str;
  if(check1) 
  {
    int check = 0;
    if(data->position == data->presNode->val->str && data->presNode->Right==NULL)
	{
      check = 1;
    }
    switch(check)
	{
      case 1:
            return RC_IM_NO_MORE_ENTRIES;

    default:
            data->presNode = data->presNode->Right;
            data->position = 0;
    }
  }
  output->page = data->presNode->L_Page->Obj[data->position];
  output->slot = data->presNode->L_Spot->Obj[data->position];
  data->position++;
  return RC_OK;
}

/**************************************************************************************
> function name: closeTreeScan
> Author: B.V. Naveena
> Email: vbathula@hawk.iit.edu
**************************************************************************************/

//closing the scan
RC closeTreeScan (BT_ScanHandle *handle)
{
  MakeEmpty(2, handle->mgmtData, handle);
  return RC_OK;
}

//Prints the whole b+tree
char *printTree (BTreeHandle *binary_tree)
{
  int cap;
  cap = binary_tree->TotalNumNodes * binary_tree->cap * 11 + binary_tree->cap + 14 + binary_tree->TotalNumNodes;
  char *output;
  output = newCharArr(cap);
  btree *node;
  node = binary_tree->root;
  int level = 0, check_leaf = 0, check = 0, j = 0;
  btree *temp = NULL;
  while(node!=NULL)
  {
    Display_Btree(node, output);
    if(node->Leaf)
	{
      check_leaf = 1;
    }

    switch(check_leaf)
	{
      case 1:
            node = node->Right;

    default:
            if(NULL == node->Right)
			{
              check = 1;
            }
            switch(check)
			{
              case 1:
                    temp = binary_tree->root;
                    while(j<=level)
					{
                      temp=temp->Child[0];
                      j++;
                    }
                    node = temp;
                    level = level + 1;

            default:
                    node = node->Right;
            }
    }
  }
  return output;
}
