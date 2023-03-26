#include "time_functions.h"

/**
	* Checks for expired deadlines in the given list and moves the corresponding tasks to the ready list.
	* @param list a pointer to the list to check
*/
void expired_deadlines(List *list){
 Node *tempNode = list->pHead;
  while(tempNode != NULL){
    if(Ticks >= tempNode->pTask->Deadline){
      PreviousTask = ReadyList->pHead->pTask;
      addAtFirstSmaller(ReadyList, deCoupler2(list, tempNode));
      NextTask = ReadyList->pHead->pTask;
      tempNode = list->pHead;
      continue;
    }
    tempNode = tempNode->pNext;
  }
}
  
/**
	* Checks if the waiting time for any task in the given list has expired and adds the corresponding task to the ready list.
	* @param list a pointer to a list
*/
void ready_for_execution(List *list){
  Node *tempNode = list->pHead;
  while(tempNode != NULL){
    if(Ticks >= tempNode->nTCnt){
      PreviousTask = ReadyList->pHead->pTask;
      addAtFirstSmaller(ReadyList, deCoupler2(list, tempNode));
      NextTask = ReadyList->pHead->pTask;
      tempNode = list->pHead;
      continue;
    }
    tempNode = tempNode->pNext;
  }
}

/**
	* Sets the current number of ticks to the given value.
	* @param nTicks the number of ticks to be set
*/
void set_ticks(uint nTicks){
  Ticks = nTicks;
}

/**
	* Returns the current number of ticks.
	* @return the current number of ticks
*/
uint ticks(void){
  return Ticks;
}

/**
	* Returns the deadline of the next task to be executed.
	* @return the deadline of the next task to be executed
*/
uint deadline(void){
  return NextTask->Deadline;
}

/**
	* Sets the deadline of the next task to be executed to the given value and adds it to the ready list.
	* @param deadline the deadline to be set
*/
void set_deadline(uint deadline){
  isr_off();
  NextTask->Deadline = deadline;
  PreviousTask = ReadyList->pHead->pTask;
  addAtFirstSmaller(ReadyList, extract(ReadyList));
  NextTask = ReadyList->pHead->pTask;
  SwitchContext();
}

/**
	* Causes the current task to wait for the given number of ticks, and adds the task to the timer list.
	* @param nTicks the number of ticks to wait for
	* @return OK if successful or DEADLINE_REACHED if the deadline has already passed.
*/
exception wait(uint nTicks){
  isr_off();
  ReadyList->pHead->nTCnt = nTicks;
  PreviousTask = ReadyList->pHead->pTask;
  addAtFirstSmaller(TimerList, extract(ReadyList));
  NextTask = ReadyList->pHead->pTask;
  SwitchContext();
  
  if(Ticks >= NextTask->Deadline)
    return DEADLINE_REACHED;
  return OK;
}

/**
	* Called when a timer interrupt is generated. 
	* Increments the tick counter, and checks for expired deadlines and tasks ready 
	* for execution in both the timer and waiting lists.
*/
void TimerInt(void){
  Ticks++;
  ready_for_execution(TimerList);
  expired_deadlines(TimerList);
  expired_deadlines(WaitingList);
}