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
 * MHT file header, 128 bytes
*/
typedef struct _MHT_FILE_HEADER {
    uchar       m_magicStr[MHT_FILE_MAGIC_STRING_LEN];
    uint32      m_rootNodeOffset;   // (RNO) in bytes
    uint32      m_firstSupplementaryLeafOffset; // (FSLO) in bytes
    uchar       m_Reserved[MHT_HEADER_RSVD_SIZE];   // 128 - 24 = 104
} MHT_FILE_HEADER, *PMHT_FILE_HEADER;

/*
70 bytes.
----------------------------------------------------------------------------
| PN | NL | HSH | ISN | IZN | LCPN | LCOS | RCPN | RCOS | PPN | POS | RSVD |
|  4 |  4 |  32 |  1  |  1  |  4   |   4  |   4  |   4  |  4  |  4  |   4  |
----------------------------------------------------------------------------
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
    uchar   m_Reserved[MHT_BLOCK_RSVD_SIZE];
} MHT_BLOCK, *PMHT_BLOCK;

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
 * Freeing an MHT block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:09:14+0800
 * @param    pmht_block               [A 2-d pointer to the MHT block to be freed]
 */
void freeMHTBlock(PMHT_BLOCK *pmht_block);

/*----------  MHT file functions  ---------------*/

/**
 * @brief      Makes an MHT file header.
 *
 * @return     The new created MHT file pointer.
 */
PMHT_FILE_HEADER makeMHTFileHeader();

/**
 * Freeing an MHT file header structure pointer.
 * @Author   DiLu
 * @DateTime 2021-11-19T15:31:07+0800
 * @param    pmht_file_header         [A 2-d pointer to the MHT file header to be freed]
 */
void freeMHTFileHeader(PMHT_FILE_HEADER *pmht_file_header);

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

/**
 * @brief      Initializing opening MHT file for reading and writing.
 *
 * @param      pathname  The path name
 *
 * @return     The file descriptor refering to the opened MHT file.
 */
int initOpenMHTFileWR(uchar *pathname);

/**
 * @brief      Reading MHT file header and returning a new created pointer to the file header structure.
 *
 * @return     The new created pointer to the file header structure.
 */
PMHT_FILE_HEADER readMHTFileHeader();

/**
 * @brief      Searching the corresponding page block in MHT file based on given page number.
 *
 * @param[in]  page_no  The page number
 *
 * @return     A new created pointer to an MHT block structure that preserving the found page block.
 *             Null will be returned if errors occur or no page is found.
 */
PMHT_BLOCK searchPageByNo(int page_no);

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
 * @brief      Serializing an MHT file header into a memory buffer.
 *
 * @param[in]  pmht_file_header  A pointer to MHT file header
 * @param      header_buf        A 2-d pointer to the output memory buffer
 * @param[in]  header_buf_len    The buffer length
 *
 * @return     How many bytes has been processed.
 */
int serialize_mht_file_header(PMHT_FILE_HEADER pmht_file_header, 
                            uchar **header_buf,
                            uint32 header_buf_len);

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
 * Converting a QNode structure into MHT block structure.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:24+0800
 * @param    qnode_ptr                [A pointer to QNode structure]
 * @param    mhtblk_ptr               [A 2-d pointer to the MHT block structure]
 * @return                            [Status of function execution. 0 for success, other values for failures.]
 */
int convert_qnode_to_mht_block(PQNode qnode_ptr, PMHT_BLOCK *mhtblk_ptr);

void *get_section_addr_in_mht_block_buffer(uchar *mht_blk_buffer, uint32 mht_blk_buffer_len, uint32 offset);

bool is_valid_offset_in_mht_block_buffer(uint32 offset);

/**
 * Printing a QNode structure for debugging.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:35+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 */
void print_qnode_info(PQNode qnode_ptr);

/*----------  File Operation Functions  ------------*/
int fo_create_mhtfile(const char *pathname);

int fo_open_mhtfile(const char *pathname);

ssize_t fo_read_mht_file_header(int fd, uchar *buffer, uint32 buffer_len);

ssize_t fo_update_mht_file_header(int fd, uchar *buffer, uint32 buffer_len);

ssize_t fo_read_mht_block(int fd, 
							uchar *buffer, 
							uint32 buffer_len, 
							int rel_distance,    // number of blocks from whence
							int whence);

ssize_t fo_read_mht_block2(int fd, 
                            uchar *buffer, 
                            uint32 buffer_len, 
                            int offset,         // number of bytes from whence
                            int whence);


ssize_t fo_update_mht_block(int fd, 
							uchar *buffer, 
							uint32 buffer_len, 
							int rel_distance, 
							int whence);

off_t fo_locate_mht_pos(int fd, off_t offset, int whence);

int fo_close_mhtfile(int fd);

#endif