#include "system_sam3x.h"
#include "at91sam3x8.h"
#include "kernel_functions.h"
#include "DoubleLinkedList.h"
#include "mail_functions.h"

// Pointers to the different lists in use in the system.
List *ReadyList = NULL; 
List *WaitingList = NULL; 
List *TimerList = NULL;

// Pointers to the previous and next running tasks in the system.
TCB *PreviousTask = NULL;
TCB *NextTask = NULL; 

// The global system tick counter.
int Ticks; 

// The current mode of the system.
int KernelMode; 

/**
	* Initializes the kernel and its data structures, including ReadyList, WaitingList, and TimerList, and leaves the kernel in start-up mode.
	* This function must be called before any other call to the kernel.
	* @return The function's status, either FAIL or OK (defined as constants in kernel.h).
	* @throws An exception if there is an error initializing the kernel.
*/
exception init_kernel(void){
    Ticks = 0;
    ReadyList = Create_List();
    if(ReadyList == NULL){
        free(ReadyList);
        return FAIL;
    }
    WaitingList = Create_List();
    if(WaitingList == NULL){
        free(ReadyList);
        free(WaitingList);
        return FAIL;
    }
    TimerList = Create_List();
    if(TimerList == NULL){
        free(ReadyList);
        free(WaitingList);
        free(TimerList);
        return FAIL;
    }
		
    if(!create_task(Idle_Task, UINT_MAX ))
        return FAIL;
    KernelMode = INIT;
    return OK;
}

/**
	* The Idle_Task function represents an empty task that runs forever. This task is typically
	* used as a placeholder for unused CPU cycles and helps prevent the system from stalling
	* due to a lack of active tasks. The Idle_Task function does not perform any operations and
	* simply enters an infinite loop, waiting for the next tick interrupt to occur.
	* @param None
	* @return None
*/
void Idle_Task(){
  while(1); // Alles Gut!
}

/**
	* Creates a task with the specified function pointer and deadline. If the call is made in start-up mode,
	* only the necessary data structures will be created. However, if the call is made in running mode,
	* it will lead to a rescheduling and possibly a context switch.

	* @param task_body A pointer to the C function holding the code of the task.
	* @param deadline The kernel will try to schedule the task so it will meet this deadline.
	* @return The exception status of the function, i.e. FAIL or OK.
*/
exception create_task (void (*taskBody)(), uint deadline) {
    TCB *new_tcb;
    new_tcb = (TCB *) calloc(1, sizeof(TCB));
    if (new_tcb == NULL) {
      free(new_tcb);
      return FAIL;
    }

    /* you must check if calloc was successful or not! */
    new_tcb->PC = taskBody;
    new_tcb->SPSR = 0x21000000;
    new_tcb->Deadline = deadline;
    new_tcb->StackSeg [STACK_SIZE - 2] = 0x21000000;
    new_tcb->StackSeg [STACK_SIZE - 3] = (unsigned int) taskBody;
    new_tcb->SP = &(new_tcb->StackSeg [STACK_SIZE - 9]);
		
    // after the mandatory initialization you can implement the rest of the suggested pseudocode
    if(KernelMode == INIT){
        exception e = addAtFirstSmaller(ReadyList, createNode(new_tcb, 0, NULL));
        if(e != OK)
            return FAIL;
        return OK;

    }else{
        isr_off();
        PreviousTask = ReadyList->pHead->pTask;
        exception e = addAtFirstSmaller(ReadyList, createNode(new_tcb, 0, NULL));
        if(e != OK)
            return FAIL;
        NextTask = ReadyList->pHead->pTask;
        SwitchContext();
    }
    return OK;
}

/**
	* Starts the kernel and the system of created tasks.
	* Control is passed to the task with the tightest deadline.
	* This function must be placed last in the application initialization code.
*/
void run (void){
	Ticks = 0;
	KernelMode = RUNNING;
	NextTask = ReadyList->pHead->pTask;

	LoadContext_In_Run();
}

/** 
	* Stop the current task and remove all of its data. Before removing the data, 
	* it switches to the process stack of the task to be called. 
	* Then, it schedules another task to start running.
*/
void terminate (void){
	isr_off();
	//REMOVE and extract the removed node without freeing memory
	Node *leavingObj = extract(ReadyList);
	NextTask = ReadyList->pHead->pTask;

  switch_to_stack_of_next_task();
  
  //remove data structers of task being terminated
  free(leavingObj->pTask);
  free(leavingObj);
  
  //Load context
  LoadContext_In_Terminate();
}


/**
	* Creates a mailbox with a specified maximum number of messages and data size for each message.
	* The mailbox is used for asynchronous and synchronous communication between tasks.
	* @param nMessages Maximum number of messages the mailbox can hold.
	* @param nDataSize Size of one message in the mailbox.
	* @return A pointer to the created mailbox or NULL if creation fails.
*/
mailbox* create_mailbox (uint nMessages, uint nDataSize) {
  mailbox* mBox = (mailbox*) calloc(1, sizeof(mailbox));
	
	// Memory Full
	if (mBox == NULL){
		free(mBox);
		return NULL;
	}
	mBox->nMaxMessages = nMessages;
	mBox->nDataSize = nDataSize;
	mBox->pHead = NULL;
	mBox->pTail = NULL;
	
	return mBox;
}

/**
	* Removes a mailbox if it is empty.
	* @param mBox Pointer to the mailbox to be removed.
	* @return OK if the mailbox was removed, NOT_EMPTY if the mailbox was not removed because it was not empty.
*/
exception remove_mailbox (mailbox* mBox){
	// Empty mailbox
	if (mBox->pHead == NULL){
		free(mBox);
		return OK;
	}else
		return NOT_EMPTY;
	
}

/**
	* Sends a message to the specified mailbox. If there is a receiving task waiting for a message on the specified mailbox,
	* the message will be delivered and the receiving task will be moved to the ReadyList. Otherwise, the sending task will be blocked.
	* During the blocking period of the task, its deadline might be reached, in which case it will be resumed with the DEADLINE_REACHED exception.
	* @param mBox Pointer to the specified mailbox.
	* @param pData Pointer to a memory area where the data of the communicated message is residing.
	* @return OK if the message was sent successfully, 
	* 				DEADLINE_REACHED if the sending task's deadline is reached while it is blocked by the send_wait call.
*/
exception send_wait(mailbox* mBox, void* pData){
	if (mBox == NULL || pData == NULL)
		return FAIL;
	
	// #Disable interrupt
	isr_off();
	
  msg* receiving_task = mBox->pHead;
	int dataSize = mBox->nDataSize;
	
	// We are the Receiver of a message
	if (receiving_task->Status == RECEIVER){
		// #Copy sender's data to the data area of the receivers Message
		memcpy(receiving_task->pData, pData, dataSize);
		// #Remove receiving task's Message struct from the mailbox
		deCoupleMessage(mBox, receiving_task);
		// #Update PreviousTask
    PreviousTask = ReadyList->pHead->pTask;
		// #Insert receiving task in ReadyList
		addAtFirstSmaller(ReadyList, deCoupler2(WaitingList, receiving_task->pBlock));
		// Decrement number of blocked messages
  	(mBox->nBlockedMsg)--;
		// #Update NextTask
    NextTask = ReadyList->pHead->pTask;
		
	// We are the Sender of a message
	}else{
		// #Allocate a Message structure Set data pointer
    msg* message = (msg*) calloc(1, sizeof(msg));
		// Memory Full check
		if (message == NULL){
			free(message);
			return FAIL;
		}
		
		// Allocate memory for the block
		message->pData = (char *) calloc(1, mBox->nDataSize);
		// Memory Full check
		if (message->pData == NULL){
			free(message->pData);
			free(message);
			return FAIL;
		}
		
		// #Add Message to the mailbox
    memcpy(message->pData, pData, mBox->nDataSize);
		
		// Setup the message struct correctly
		message->Status = SENDER;		
		message->pBlock = ReadyList->pHead;
			
		// Add message to mailbox
    addMessage(mBox, message);
		// Increment number of blocked messages
  	(mBox->nBlockedMsg)++;
		
		// #Update PreviousTask
    PreviousTask = ReadyList->pHead->pTask;
		// #Move sending task from ReadyList to WaitingList
		addAtFirstSmaller(WaitingList, deCoupler2(ReadyList, message->pBlock));
		// #Update NextTask
    NextTask = ReadyList ->pHead ->pTask;
	}
	
	SwitchContext();
	
	if (Ticks >= NextTask->Deadline){
		isr_off();
		// Remove SEND message
		msg* e_message = extractMessage(mBox);
                (mBox->nBlockedMsg)--;
		// Free as we are now done
		//free(e_message); **********************************
		// Interrupts on again
		isr_on();
		return DEADLINE_REACHED;
	}else
		return OK;
}

/**
	* Sends a Message to the specified mailbox. The sending task will continue execution after the call.
	* When the mailbox is full, the oldest Message will be overwritten. The send_no_wait call will imply a
	* new scheduling and possibly a context switch. Note: send_wait and send_no_waitMessages shall not be mixed
	* in the same mailbox.
	* @param mBox a pointer to the specified Mailbox.
	* @param pData a pointer to a memory area where the data of the communicated Message is residing.
	* @return Description of the function's status, i.e. FAIL/OK.
*/
exception send_no_wait(mailbox* mBox, void* pData){
	if (mBox == NULL || pData == NULL)
		return FAIL;
	
  // #Disable interrupt
  isr_off();
	
  msg* receiving_task = mBox->pHead;
	int dSize = mBox->nDataSize;
	
	// We are the Receiver of a message
	if (receiving_task->Status == RECEIVER){
		// #Copy sender's data to the data area of the receivers Message
		memcpy(receiving_task->pData, pData, dSize);
		// #Remove receiving task's Message struct from the mailbox
		deCoupleMessage(mBox, receiving_task);
		// #Update PreviousTask
    PreviousTask = ReadyList->pHead->pTask;
		// #Insert receiving task in ReadyList
		addAtFirstSmaller(ReadyList, deCoupler2(WaitingList, receiving_task->pBlock));
		// #Update NextTask
    NextTask = ReadyList->pHead->pTask;
		// #Switch context
		SwitchContext();
	}else{
		// #Allocate a Message structure Set data pointer
    msg* message = (msg*) calloc(1, sizeof(msg));
		
		// Memory Full check
		if(message == NULL){
			msg* e_message = extractMessage(mBox);
			free(e_message->pData);
			free(e_message);
    	msg* message = (msg*) calloc(1, sizeof(msg));
		}
		// Allocate memory for the block
		message->pData = (char *) calloc(1, mBox->nDataSize);
		
		// Memory Full check
		if (message->pData == NULL){
			free(message->pData);
			free(message);
			return FAIL;
		}
		
		// #Copy data to the Message
		memcpy(message->pData, pData, dSize);
		// Setup the message struct correctly
		message->pBlock = NULL;
		message->Status = SENDER;
		
		if (mBox->nMessages >= mBox->nMaxMessages){
			msg* e_message = extractMessage(mBox);
			free(e_message->pData);
			free(e_message);
		}
		
		addMessage(mBox, message);
		// No SwitchContext function call so has to enable Interrupt manually again
		isr_on();
	}
		return OK;
}

/**
	* Attempts to receive a message from the specified mailbox. If there is a send_wait or a send_no_wait message waiting for a receive_wait or a
	* receive_no_wait message on the specified mailbox, receive_wait will collect it. If the message was of send_wait type, the sending task
	* will be moved to the ReadyList. Otherwise, if there is no send message waiting on the specified mailbox, the receiving task will be blocked.
	* During the blocking period of the task, its deadline might be reached, in which case it will be resumed with the DEADLINE_REACHED exception.
	* @param mBox Pointer to the specified mailbox.
	* @param pData Pointer to a memory area where the data of the communicated message is to be stored.
	* @return OK if the message was received successfully, DEADLINE_REACHED if the receiving task's deadline is reached while it is blocked by the receive_wait call.
*/
exception receive_wait(mailbox* mBox, void* pData){
	if (mBox == NULL || pData == NULL)
		return FAIL;
	
	// #Disable interrupt
	isr_off();
	
	msg* sending_message = mBox->pHead;
	int dSize = mBox->nDataSize;
	
	if (sending_message->Status == SENDER){
		// # Copy sender's data to receiving task's data area
		memcpy(pData, sending_message->pData, dSize);
		// Remove sending task's Message struct from the mailbox
		deCoupleMessage(mBox, sending_message);
		
		if (isMember(WaitingList, sending_message->pBlock)){
			// #Update PreviousTask
			PreviousTask = ReadyList->pHead->pTask;
			// #Move sending task to ReadyList
			addAtFirstSmaller(ReadyList, deCoupler2(WaitingList, sending_message->pBlock));
			// #Update NextTask
			NextTask = ReadyList ->pHead ->pTask;	
		}else{
			// #Free senders data area
			free(sending_message->pData);
			//free(sending_message); ??
		}
  	(mBox->nBlockedMsg)--;
	}else{
		
		// #Allocate a Message structure
    msg* message = (msg*) calloc(1, sizeof(msg));
		
		// Memory Full check
		if (message == NULL){
			free(message);
			return FAIL;
		}
		// Allocate memory for the block
		message->pData = (char *) calloc(1, mBox->nDataSize);
		
		// Memory Full check
		if (message->pData == NULL){
			free(message->pData);
			free(message);
			return FAIL;
		}
		
		// #Add Message to the mailbox
		message->pData = pData;
		message->Status = RECEIVER;
		message->pBlock = ReadyList->pHead;
		ReadyList->pHead->pMessage = message;
		
  	addMessage(mBox, message);
		
		// #Update PreviousTask
    PreviousTask = ReadyList->pHead->pTask;
		// #Move sending task from ReadyList to WaitingList
		addAtFirstSmaller(WaitingList, deCoupler2(ReadyList, message->pBlock));
		// Increment number of blocked messages
  	(mBox->nBlockedMsg)++;
		// #Update NextTask
    NextTask = ReadyList ->pHead ->pTask;
	}
	SwitchContext();
	
	if (Ticks >= NextTask->Deadline){
		isr_off();
		// Remove SEND message
		msg* e_message = deCoupleMessage(mBox, ReadyList->pHead->pMessage);
		// Free as we are now done
		//free(e_message); **********************************
		isr_on();
		return DEADLINE_REACHED;
	}else
		return OK;
}

/**
	* Attempts to receive a Message from the specified mailbox. The calling task will continue execution after the call.
	* When there is no sendMessage available, or if the mailbox is empty, the function will return FAIL. Otherwise, OK
	* is returned. The call might imply a new scheduling and possibly a context switch.
	* @param mBox a pointer to the specified mailbox.
	* @param pData a pointer to the Message.
	* @return Integer indicating whether a Message was received or not (OK/FAIL).
*/
exception receive_no_wait(mailbox* mBox, void* pData){
	if (mBox == NULL || pData == NULL)
		return FAIL;
	
	msg* sending_message = mBox->pHead;
	int dSize = mBox->nDataSize;
	
	
	if (sending_message->Status == SENDER){
		// #Disable interrupt
		isr_off();
		// # Copy sender's data to receiving task's data area
		memcpy(pData, sending_message->pData, dSize);
		// Remove sending task's Message struct from the mailbox
                
    // Free the message struct
		deCoupleMessage(mBox, sending_message);
		
		if (isMember(WaitingList, sending_message->pBlock)){
			// #Update PreviousTask
			PreviousTask = ReadyList->pHead->pTask;
			// #Move sending task to ReadyList
			addAtFirstSmaller(ReadyList, deCoupler2(WaitingList, sending_message->pBlock));
			mBox->nBlockedMsg--;
			// #Update NextTask
			NextTask = ReadyList ->pHead ->pTask;		
      SwitchContext();	
		}else{
			// #Free senders data area
			free(sending_message->pData);
		}
	}else
		return FAIL;
	
	return OK;
}



