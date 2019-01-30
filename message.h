// --------------------------------
// Assignment  : PA02-IPC
// Date        : 10/2/18
//
// Author      : Nathan Chan and Amanda Pearce
// File Name   : message.h
// ----------------------------------
#include <sys/types.h>

typedef struct {

	long msgType ; /* 1: request , 2: reply */
	struct {
	    int production;           /* 0 for completion message, 1 for production message */
		int factory_ID;		/* Factory Line_ID, a unique integer from 1 to N (number of factory lines) */
		int capacity; 		/* the maximum number of parts it can make in one iteration */
		int duration;		/* The time in milliseconds to make that many parts */
		int iterations;		/* For the completion message */
		int parts_made;		/* For the completion message */
	} body;
} msgBuf ;

#define MSG_INFO_SIZE ( sizeof(msgBuf) - sizeof(long) ) // may not need this either

void printMsg( msgBuf *m ) ;

