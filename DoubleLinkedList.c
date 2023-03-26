#include "DoubleLinkedList.h"

/**
	* Create a new empty doubly linked list.
	* @return A pointer to the newly created list.
*/
struct _list* Create_List(){
    struct _list* list = NULL;
    list = (struct _list *) calloc(1, sizeof(struct _list));
    if (list == NULL)
        return NULL;
    return list;
}

/**
	* Create a new node to store a task and a message.
	* @param pTask A pointer to the task to be stored in the node.
	* @param nTCnt The count of the task.
	* @param pMessage A pointer to the message to be stored in the node.
	* @return A pointer to the newly created node.
*/
Node *createNode(TCB *pTask, uint nTCnt, msg *pMessage){
    Node *p = NULL;
    p = (Node *)calloc(1, sizeof(Node));
    if (p == NULL) 
        return NULL;
    
    p->pTask = pTask;
    p->nTCnt = nTCnt;
    p->pMessage = pMessage;
    p->pNext = NULL;
    p->pPrevious = NULL;
    return p;
}

/**
	* Add a new node at the beginning of the doubly linked list if the deadline of the new task is smaller than or equal to the deadline of the first node.
	* Otherwise, traverse the list and add the node after the first node whose deadline is greater than the deadline of the new node.
	* @param list A pointer to the doubly linked list.
	* @param newNode A pointer to the new node to be added.
	* @return An exception indicating whether the operation was successful or not.
*/
exception addAtFirstSmaller(List *list, Node *newNode){
    if(list == NULL || newNode == NULL)
        return FAIL;

    //If the list is empty then create a node
    if(list->pHead == NULL){
        list->pHead = newNode;
        list->pTail = newNode;
        return OK;
    }

    if(newNode->pTask == NULL || list->pHead->pTask == NULL || list->pTail->pTask == NULL)
        return FAIL;

    //Only one node exist
    if(list->pHead == list->pTail){
        if(newNode->pTask->Deadline <= list->pHead->pTask->Deadline){
            list->pHead = newNode;
            list->pTail->pPrevious = newNode;
            list->pHead->pNext = list->pTail;
        }else{
            list->pTail = newNode;
            list->pHead->pNext = newNode;
            list->pTail->pPrevious = list->pHead;
        }
        return OK;
    }
    Node *tempNode = list->pHead;

    //Smaller than the first node
    if(newNode->pTask->Deadline < list->pHead->pTask->Deadline){
        newNode->pNext = list->pHead;
        list->pHead->pPrevious = newNode;
        list->pHead = newNode;
        return OK;
    }

    //Find a node larger than the newNode in the list
    while(tempNode->pNext != NULL){
        if(tempNode->pNext->pTask == NULL)
            return OK;
        if(newNode->pTask->Deadline  < tempNode->pNext->pTask->Deadline ){
            break;
        }
        tempNode = tempNode->pNext;
    }

    //In the middle state of the nodes.
    if(tempNode->pNext != NULL){
        tempNode->pNext->pPrevious = newNode;
        newNode->pNext = tempNode->pNext;
        newNode->pPrevious = tempNode;
        tempNode->pNext = newNode;
        return OK;
    }

    //The last node
        newNode->pPrevious = tempNode;
        tempNode->pNext = newNode;
        list->pTail = newNode;
        return OK;
}

/**
	* Extract the node with the smallest deadline from the doubly linked list.
	* @param list A pointer to the doubly linked list.
	* @return A pointer to the extracted node or NULL if the list is empty or the extracted node is the idle task.
*/
Node* extract(List *list){
	// make sure for the next statement
  if (list->pHead == NULL || list->pHead->pTask == NULL)
    return NULL;
  
	// Make sure it's not the idle task
  if(list->pHead->pTask->Deadline == UINT_MAX)
    return NULL;
    
  return deCoupler2(list, list->pHead);
}

/**
	* Remove the given node from the doubly linked list.
	* @param list A pointer to the doubly linked list.
	* @param node A pointer to the node to be removed.
	* @return A pointer to the removed node or NULL if the list is empty or the node is not in the list.
*/
Node* deCoupler2(List *list, Node *node){
	// MAKE SURE NO NULL IS SENT IN
  if (list->pHead == NULL || node == NULL)
    return NULL;
	
    //Only existing node
  if(node->pPrevious == NULL && node->pNext == NULL){
    list->pHead = NULL;
    list->pTail = NULL;
    
    //Decoupler the first node
  }else if(node->pPrevious == NULL && node->pNext != NULL){
    list->pHead = node->pNext;
    node->pNext->pPrevious = NULL;
    
  // Middle Node
  }else if(node->pNext != NULL && node->pPrevious != NULL){
    node->pPrevious->pNext = node->pNext;
    node->pNext->pPrevious = node->pPrevious;
    
  // Last Node
  }else{
    list->pTail = node->pPrevious;
    node->pPrevious->pNext = NULL;
  }
  node->pPrevious = NULL;
  node->pNext = NULL;
  return node;
}