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
/**
 * Initializing an MHT block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:06:10+0800
 * @param    pmht_block               [a pointer to MHT block structure]
 */
void initMHTBlock(PMHT_BLOCK *pmht_block);

/**
 * Creating an MHT block structure. The created object needs to be destroyed by freeMHTBlock().
 * @Author   DiLu
 * @DateTime 2021-11-10T14:06:44+0800
 * @return   [A pointer to the created MHT block structure]
 */
PMHT_BLOCK makeMHTBlock();

/**
 * Creating an MHT header block structure. The created object needs to be destroyed by freeMHTHeaderBlock().
 * @Author   DiLu
 * @DateTime 2021-11-10T14:08:03+0800
 * @return   [A pointer to the created MHT header block structure]
 */
PMHT_HEADER_BLOCK makeMHTHeaderBlock();

/**
 * Creating an MHT child node block structure. The created object needs to be destroyed by freeMHTChildNodeBlock().
 * @Author   DiLu
 * @DateTime 2021-11-10T14:08:36+0800
 * @return   [A pointer to the created MHT child node block structure]
 */
PMHT_CHILD_NODE_BLOCK makeMHTChildNodeBlock();

/**
 * Freeing an MHT block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:09:14+0800
 * @param    pmht_block               [A 2-d pointer to the MHT block to be freed]
 */
void freeMHTBlock(PMHT_BLOCK *pmht_block);

/**
 * Freeing an MHT header block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:10:04+0800
 * @param    pmht_header_block        [A 2-d pointer to the MHT header block to be freed]
 */
void freeMHTHeaderBlock(PMHT_HEADER_BLOCK *pmht_header_block);

/**
 * Freeing an MHT child node block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:10:27+0800
 * @param    pmht_child_node_block    [A 2-d pointer to the MHT child node block to be freed]
 */
void freeMHTChildNodeBlock(PMHT_CHILD_NODE_BLOCK *pmht_child_node_block);

/*----------  MHT file functions  ---------------*/
/**
 * A test for building an MHT queue. Just for testing.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:11:07+0800
 */
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
 * @brief      Serializing an MHT block into a memory buffer. 
 *
 * @param[in]  pmht_block     The MHT block
 * @param[out] block_buf      The block buffer
 * @param[in]  block_buf_len  The block buffer length
 *
 * @return     How many bytes has been processed.
 */
int serialize_mht_block(PMHT_BLOCK pmht_block, uchar **block_buf, uint32 block_buf_len);

/**
 * Serializing an MHT header block into a memory buffer.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:12:06+0800
 * @param    pmht_header_block        [A pointer to the MHT header block structure]
 * @param    block_buf                [A 2-d pointer to the buffer that is used to storing MHT header block data]
 * @param    block_buf_len            [The size of the buffer above]
 * @return                            [How many bytes has been processed.]
 */
int serialize_mhthdr_block(PMHT_HEADER_BLOCK pmht_header_block, uchar **block_buf, uint32 block_buf_len);

/**
 * Serializing an MHT child node block into a memory buffer.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:15:30+0800
 * @param    pmht_child_node_block    [A pointer to the MHT child node block structure]
 * @param    block_buf                [A 2-d pointer to the buffer that is used to storing MHT child node block data]
 * @param    block_buf_len            [The size of the buffer above]
 * @return                            [How many bytes has been processed.]
 */
int serialize_cldnode_block(PMHT_CHILD_NODE_BLOCK pmht_child_node_block, uchar **block_buf, uint32 block_buf_len);

/**
 * @brief      Restoring an MHT Block structure from a given memory buffer.
 *
 * @param[in]       block_buf      The block buffer
 * @param[in]       block_buf_len  The block buffer length
 * @param[out]      pmht_block     The MHT block
 *
 * @return     How many bytes has been processed.
 */
int unserialize_mht_block(char *block_buf, uint32 block_buf_len, PMHT_BLOCK *pmht_block);

/**
 * Directly serialzing a QNode structure into memory buffer (MHT block buffer). 
 * @Author   DiLu
 * @DateTime 2021-11-10T14:16:46+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 * @param    mht_block_buf            [A 2-d pointer to the buffer that is used to storing serialized data]
 * @param    mht_block_buf_len        [The size of the buffer above]
 * @return                            [How many bytes has been processed.]
 */
int qnode_to_mht_buffer(PQNode qnode_ptr, uchar **mht_block_buf, uint32 mht_block_buf_len);

/**
 * Directly serialzing a QNode structure into memory buffer (MHT header block buffer). 
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:16+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 * @param    mht_block_buf            [A 2-d pointer to the buffer that is used to storing serialized data]
 * @param    mht_block_buf_len        [The size of the buffer above]
 * @return                            [How many bytes has been processed.]
 */
int qnode_to_mht_header_buffer(PQNode qnode_ptr, uchar **mht_block_buf, uint32 mht_block_buf_len);

/**
 * Directly serialzing a QNode structure into memory buffer (MHT child node block buffer). 
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:16+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 * @param    mht_block_buf            [A 2-d pointer to the buffer that is used to storing serialized data]
 * @param    mht_block_buf_len        [The size of the buffer above]
 * @return                            [How many bytes has been processed.]
 */
int qnode_to_mht_cldnode_buffer(PQNode qnode_ptr, uchar **mht_block_buf, uint32 mht_block_buf_len);

/**
 * Converting a QNode structure into MHT block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:24+0800
 * @param    qnode_ptr                [A pointer to QNode structure]
 * @param    mhtblk_ptr               [A 2-d pointer to the MHT block structure]
 * @return                            [Status of function execution. 0 for success, other values for failures.]
 */
int convert_qnode_to_mht_block(PQNode qnode_ptr, PMHT_BLOCK *mhtblk_ptr);

/**
 * Converting a QNode structure into MHT header block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:24+0800
 * @param    qnode_ptr                [A pointer to QNode structure]
 * @param    mht_hdrblk_ptr               [A 2-d pointer to the MHT header block structure]
 * @return                            [Status of function execution. 0 for success, other values for failures.]
 */
int convert_qnode_to_mht_hdr_block(PQNode qnode_ptr, PMHT_HEADER_BLOCK *mht_hdrblk_ptr);

/**
 * Converting a QNode structure into MHT child node block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:24+0800
 * @param    qnode_ptr                [A pointer to QNode structure]
 * @param    mht_cldblk_ptr               [A 2-d pointer to the MHT child node block structure]
 * @return                            [Status of function execution. 0 for success, other values for failures.]
 */
int convert_qnode_to_mht_cldnode_block(PQNode qnode_ptr, PMHT_CHILD_NODE_BLOCK *mht_cldblk_ptr);

/**
 * Printing a QNode structure for debugging.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:35+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 */
void print_qnode_info(PQNode qnode_ptr);

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