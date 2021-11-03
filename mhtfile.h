#ifndef _MHTFILE
#define _MHTFILE

#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "sha256.h"


void testMHTQueue();

/*
Building MHT file from a given database file.
Parameters:
	NULL.
Return:
	NULL.
*/
void buildMHTFile();

/*----------  Helper Functions  ---------------*/

/*
This function only deals with the remaining nodes in the queue, which will 
finish building a complte MHT by using "zero node" (node with hash of 0).
Parameters:
	pQHeader: a 2-d pointer to queue's header.
	pQ: a 2-d pointer to queue's tail (current enqueued position).
Return:
	NULL.
*/
void deal_with_remaining_nodes_in_queue(PQNode *pQHeader, PQNode *pQ);

/*----------  File Operation Functions  ------------*/

#endif