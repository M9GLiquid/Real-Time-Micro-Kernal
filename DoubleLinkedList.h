#ifndef DOUBLELINKEDLIST_H
  #define DOUBLELINKEDLIST_H

  #include "kernel_functions.h"

  //Functions
  Node *createNode(TCB *pTask, uint nTCnt, msg *pMessage);
  struct _list* Create_List();
  void addLast(List *list, Node *newNode);
  exception addAtFirstSmaller(List *list, Node *newNode);
	Node* deCoupler2(List *list, Node *node);
	void deCoupler(List *list, Node *node);
	Node* extract(List *list);

#endif
