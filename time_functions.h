#ifndef TIME_FUNCTIONS_H
  #define TIME_FUNCTIONS_H

#include "DoubleLinkedList.h"
#include "kernel_functions.h"

void set_ticks(uint nTicks);
uint ticks(void);
uint deadline(void);
void set_deadline(uint deadline);
exception wait(uint nTicks);
void TimerInt(void);

void expired_deadlines(List *list); //extra function
void ready_for_execution(List *list); //extra function

#endif
