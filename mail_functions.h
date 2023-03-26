#ifndef MAIL_FUNCTIONS_H
  #define MAIL_FUNCTIONS_H
		
  #include "kernel_functions.h"
	
  //Functions
	msg* extractMessage(mailbox* mBox);
	msg* deCoupleMessage(mailbox* mBox, msg* message);
	void addMessage(mailbox* mBox, msg* message);
	bool isMember(List* list, Node* match);

#endif