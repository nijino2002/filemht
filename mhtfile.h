#ifndef _MHTFILE
#define _MHTFILE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "sha256.h"

/*
66 bytes.
---------------------------------------------------------------------
| PN | NL | HSH | ISN | IZN | LCPN | LCOS | RCPN | RCOS | PPN | POS |
|  4 |  4 |  32 |  1  |  1  |  4   |   4  |   4  |   4  |  4  |  4  |
---------------------------------------------------------------------
*/
typedef struct _MHT_BLOCK {
	int		m_pageNo;
	int 	m_nodeLevel;
	char	m_hash[HASH_LEN];
	uchar 	m_isSupplementaryNode;
	uchar	m_isZeroNode;
	int 	m_lChildPageNo;
	int 	m_lChildOffset;
	int 	m_rChildPageNo;
	int 	m_rChildOffset;
	int 	m_parentPageNo;
	int 	m_parentOffset;
} MHT_BLOCK, *PMHT_BLOCK;

/*
--------------------
| MHT BLOCK | RSVD |
|     66    |  62  |
--------------------
*/
typedef struct _MHT_HEADER_BLOCK {
	MHT_BLOCK m_nodeBlock;
	char 	m_Reserved[MHT_HEADER_RSVD_SIZE];
} MHT_HEADER_BLOCK, *PMHT_HEADER_BLOCK;

/*
--------------------
| MHT BLOCK | RSVD |
|     66    |   4  |
--------------------
*/
typedef struct _MHT_CHILD_NODE_BLOCK{
	MHT_BLOCK m_nodeBlock;
	char 	m_Reserved[MHT_CNB_RSVD_SIZE];
} MHT_CHILD_NODE_BLOCK, *PMHT_CHILD_NODE_BLOCK;

/*-------------  MHT block processing functions  --------------*/
void initMHTBlock(PMHT_BLOCK *pmht_block);

PMHT_BLOCK makeMHTBlock();

PMHT_HEADER_BLOCK makeMHTHeaderBlock();

PMHT_CHILD_NODE_BLOCK makeMHTChildNodeBlock();

void freeMHTBlock(PMHT_BLOCK *pmht_block);

void freeMHTHeaderBlock(PMHT_HEADER_BLOCK *pmht_header_block);

void freeMHTChildNodeBlock(PMHT_CHILD_NODE_BLOCK *pmht_child_node_block);

/*----------  MHT file functions  ---------------*/
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
 * @brief      { This function deals with the page numbers of the interior nodes, 
 * 				which can implement binary search for the MHT. }
 *
 * @param[in]  parent_ptr  The parent pointer generated by the combination of two children.
 * @param[in]  lchild_ptr  The lchild pointer
 * @param[in]  rchild_ptr  The rchild pointer
 */
void deal_with_interior_nodes_pageno(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr);

/**
 * @brief      { Serializing a MHT block into a memory buffer. }
 *
 * @param[in]  pmht_block     The MHT block
 * @param[out] block_buf      The block buffer
 * @param[in]  block_buf_len  The block buffer length
 *
 * @return     { description_of_the_return_value }
 */
int serialize_mht_block(PMHT_BLOCK pmht_block, uchar **block_buf, uint32 block_buf_len);

int serialize_mhthdr_block(PMHT_HEADER_BLOCK pmht_header_block, uchar **block_buf, uint32 block_buf_len);

int serialize_cldnode_block(PMHT_CHILD_NODE_BLOCK pmht_child_node_block, uchar **block_buf, uint32 block_buf_len);

/**
 * @brief      { Building an MHT Block structure from a given memory buffer. }
 *
 * @param[in]       block_buf      The block buffer
 * @param[in]       block_buf_len  The block buffer length
 * @param[out]      pmht_block     The MHT block
 *
 * @return     { description_of_the_return_value }
 */
int unserialize_mht_block(char *block_buf, uint32 block_buf_len, PMHT_BLOCK *pmht_block);

int convert_qnode_to_mht_block(PQNode qnode_ptr, PMHT_BLOCK *mhtblk_ptr);

int convert_qnode_to_mht_hdr_block(PQNode qnode_ptr, PMHT_HEADER_BLOCK *mht_hdrblk_ptr);

int convert_qnode_to_mht_cldnode_block(PQNode qnode_ptr, PMHT_CHILD_NODE_BLOCK *mht_cldblk_ptr);

int convert_mht_block_to_qnode(PMHT_BLOCK mhtblk_ptr, PQNode *qnode_ptr);

int convert_mht_cldnode_block_to_qnode(PMHT_CHILD_NODE_BLOCK mht_hdrblk_ptr, PQNode *qnode_ptr);

int convert_mht_hdr_block_to_qnode(PMHT_HEADER_BLOCK mht_hdrblk_ptr, PQNode *qnode_ptr);

/*----------  File Operation Functions  ------------*/
int fo_create_mhtfile(const char pathname);

int fo_open_mhtfile(const char pathname);

ssize_t fo_read_mht_header_block();

ssize_t fo_update_mht_header_block();

ssize_t fo_read_mht_child_node_block();

ssize_t fo_update_mht_child_node_block();

off_t fo_locate_mht_pos();

int fo_close_mhtfile();

#endif