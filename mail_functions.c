#include "mail_functions.h"

/**
	* Extracts the message from the mailbox and decouples it from the linked list.
	* @param mBox the mailbox containing the message to be extracted
	* @return the extracted message or NULL if the mailbox or the message is empty
*/
msg* extractMessage(mailbox *mBox){
  return deCoupleMessage(mBox, mBox->pHead);
}

/**
	* Decouples a message from the linked list of a mailbox.
	* @param mBox the mailbox containing the message to be decoupled
	* @param message the message to be decoupled
	* @return the decoupled message or NULL if the mailbox or the message is empty
*/
msg* deCoupleMessage(mailbox* mBox, msg* message){
	// MAKE SURE NO NULL IS SENT IN
  if (mBox->pHead == NULL || message == NULL)
    return NULL;
	
    //Only existing message
  if(message->pPrevious == NULL && message->pNext == NULL){
    message = NULL;
    mBox->pHead = NULL;
    mBox->pTail = NULL;
    
    //Decoupler the first message
  }else if(message->pPrevious == NULL){
    mBox->pHead = message->pNext;
    message->pNext->pPrevious = NULL;
    
    //Decoupler the last message
  }else if(message->pPrevious == NULL){
    message->pPrevious->pNext = NULL;
    mBox->pTail = message->pPrevious;

    //Decoupler the middle message
  }else if(message->pPrevious == NULL){
    message->pPrevious->pNext = message->pNext;
    message->pNext->pPrevious = message->pPrevious;
  }
  message->pPrevious = NULL;
  message->pNext = NULL;
	// Decrement number of messages
  (mBox->nMessages)--;
  return message;
}

/**
	* Adds a message to the end of the linked list of a mailbox.
	* @param mBox the mailbox to which the message will be added
	* @param message the message to be added
*/
void addMessage(mailbox *mBox, msg* message){
	// No Message in mailbox
  if (mBox->pHead == NULL) {
    mBox->pHead = message;
    mBox->pTail = message;
		
	// Message(s) in the mailbox
  }else {
    message->pPrevious = mBox->pTail;
    mBox->pTail->pNext = message;
    mBox->pTail = message;
  }
	
	// Increment number of messages
  (mBox->nMessages)++;
}
	
/**
	* Checks if a given node is a member of a linked list.
	* @param list the linked list to be searched
	* @param match the node to be matched
	* @return TRUE if the node is found in the list, FALSE otherwise
*/
bool isMember(List* list, Node* match){
	if(list == NULL || match == NULL || list->pHead == NULL || list->pTail == NULL)
		return FAIL;
  
	Node* current_message = list->pHead;
	
	// Match in the beginning
	if (current_message == match)
		return TRUE;
	
	// Match in the middle
	while (current_message->pNext != NULL){
  	if (current_message == match)
  		return TRUE;
  	current_message = current_message->pNext;
  }
	
	// Match in the end
	if (current_message == match)
		return TRUE;
	
	// No match
	return FALSE;
}


