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
This function deals with each page of the database file and create a MHT sequence
in a double-linked queue.
Parameters:
	pQHeader: a 2-d pointer to queue's header.
	pQ: a 2-d pointer to queue's tail (current enqueued position).
Return:
	NULL.
*/
void process_all_pages(PQNode *pQHeader, PQNode *pQ);

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

/*
Calculating the relative distance between two nodes according to the point of view (pov).
By default, qnode1_ptr is prior to qnode2_ptr.
Parameters:
	qnode1_ptr: the pointer to node1.
	qnode2_ptr: the pointer to node2.
	pov: point of view. 0x01 refers to node1, and 0x02 refers to node2.
*/
uint32 compute_relative_distance_between_2_nodes(PQNode qnode1_ptr, 
												 PQNode qnode2_ptr,
												 uchar pov);

/**
 * [deal_with_nodes_offset description]
 * Dealing with nodes offset (relative distance), which will be used in creating MHT file.
 * @Author   DiLu
 * @DateTime 2021-11-05T16:44:14+0800
 * @param    parent_ptr               [Parent node pointer, which usually refers to the node having been combined by two children]
 * @param    lchild_ptr               [Left child node pointer]
 * @param    rchild_ptr               [Right child node pointer]
 */
void deal_with_nodes_offset(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr);

/**
 * [deal_with_interior_nodes_pageno description]
 * This function deals with the page numbers of the interior nodes, which can implement binary search for the MHT.
 * @Author   DiLu
 * @DateTime 2021-11-05T17:21:15+0800
 * @param    parent_ptr               [The parent node pointer when two-node combination finishes]
 */
void deal_with_interior_nodes_pageno(PQNode parent_ptr);

/*----------  File Operation Functions  ------------*/

#endif