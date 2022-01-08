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
PMHT_FILE_HEADER readMHTFileHeader(int fd);

/**
 * @brief      Searching the corresponding page block in MHT file based on given page number.
 *
 * @param[in]  fd       The file descriptor.
 * @param[in]  page_no  The page number
 *
 * @return     A new created pointer to an MHT block structure that preserving the found page block.
 *             Null will be returned if errors occur or no page is found.
 */
PMHT_BLOCK searchPageByNo(int fd, int page_no);

/**
 * @brief      Locates an MHT block offset by page number.
 *
 * @param[in]  fd       The file descriptor.
 * @param[in]  page_no  The given page number.
 *
 * @return     If success, the offset of the block corresponding to the given page number is returned,
 *             otherwise, values <= 0 will be returned.
 */
int locateMHTBlockOffsetByPageNo(int fd, int page_no);

/*
更新某个MHT节点到根的路径上的节点的哈希值
Parameters:
		update_block_buf: 被更新的页节点块信息.
		update_blobk_offset:被更新的页节点块偏移量
		fd: 文件描述符
returns:
		如果更新失败，返回值小于0。
*/
/**
 * @brief  		   Update the hash value of the node on the path from a certain MHT node to the root
 *
 * 	@param[in]		update_block_buf: 			 A pointer to the updated node block information.
 *	@param[in]		update_blobk_offset:		 The offset of the updated node block in the file.
 *	@param[in] 		fd:					         The file descriptor.
 *
 * 	@return		  		If fails,values <= 0 will be returned.
*/
int updatePathToRoot(uchar *update_block_buf, int update_blobk_offset, int fd);

/**
 * @brief      Update the hash value of an MHT block corresponding to the given page number.
 *
 * @param[in]  page_no       The given page number
 * @param      hash_val      The new hash value
 * @param[in]  hash_val_len  The new hash value length
 * @param[in]  fd		     The file descriptor
 *
 * @return     If success, the offset of the block that has been updated is returned, 
 *             otherwise, values <= 0 will be returned.
 */
int updateMHTBlockHashByPageNo(int page_no, uchar *hash_val, uint32 hash_val_len, int fd);

/**
Update the MHT information according to the given node block information
Parameters:
		mhtblk_buffer: 被更新的页节点块信息.
		blobk_offset:被更新的页节点块偏移量
		fd: 文件描述符
returns:
		如果更新失败，返回值小于0。
 */
/**
 *  @brief					  Update the MHT information according to the given node block information
 *
 * 	@param[in] 			mhtblk_buffer: 		 A pointer to the updated node block information.  
 *	@param[in] 			blobk_offset:		 The offset of the updated node block in the file
 *	@param[in] 			fd:					 The file descriptor
 *
 *  @return					If fails,values <= 0 will be returned.
*/
int updateMHTBlockHashByMHTBlock(uchar *mhtblk_buffer, int blobk_offset, int fd);

/**
 * @brief      Insert a new MHT block to MHT file. Note that this will occur when a new page has been generated by 
 *         the database. According to our design, the new block related to the new page will replace the first 
 *         supplementary block in the MHT file. After replacement, all the page numbers and hash values of the nodes 
 *         in the path from the new block (node) to the root will be updated. If there is no supplementary block in 
 *         the MHT file, the new block will be inserted before root block, and the MHT will be adjusted to a new 
 *         complete binary tree via adding new supplementary blocks (nodes).
 *
 * @param[in]  pmht_block   A pointer to an MHT block structure corresponding to the new page. 
 * @param[in]	fd                      The file descriptor
 *
 * @return     If success, the offset of the new block that has been inserted is returned, 
 *             otherwise, values <= 0 will be returned.
 */
int insertNewMHTBlock(PMHT_BLOCK pmht_block, int fd);

/*
    往MHT文件中乱序插入一个页面，要求插入后MHT仍满足二叉平衡搜索树的特性。先判断页面是否存在，如果存在则进行更新操作，否则进行下一步操作。首先，
    将文件原始信息拷贝到另一个辅助文件中。读取文件信息来确定该文件是否还有可填充节点，若没有，我们需要先对原始文件的MHT进行扩充，因为当原始MHT是一
    个完全二叉树时，插入一页会出现悬空节点。接下来，在原始文件中找到要插入的位置，用page_no和hash_val生成新的节点信息去替代当前节点信息，同时完
    成相应信息的更新。然后将剩余的节点信息从辅助文件中读出，覆盖回写至原始文件即可，直至两种文件中某一个文件结束或者辅助文件中剩下节点为填充节点时，
    最后更新文件头信息即可。
Parameters:
		page_no:        被插入的页面的页码值
		hash_val:       被插入的页面对应的哈希值
		hash_val_len:   哈希值的长度
returns:
		如果插入失败，返回值小于0。
*/
/**
 * @brief     Inserting a page out of order into the MHT file requires that the MHT still satisfies the characteristics
 *            of a binary balanced search tree after insertion. First determine whether the page exists, and if so,
 *            perform the update operation, otherwise, proceed to the next step. First, copy the original information of
 *            the file into another auxiliary file. Read the file information to determine whether the file has nodes
 *            that can be filled. If not, we need to expand the MHT of the original file, because when the original MHT
 *            is a complete binary tree, inserting a page will have dangling nodes. Next, find the position to be
 *            inserted in the original file, use page_no and hash_val to generate new node information to replace the
 *            current node information, and complete the update of the corresponding information. Then read the remaining
 *            node information from the auxiliary file, overwrite and write back to the original file, until one of the
 *            two files ends or the remaining nodes in the auxiliary file are filled nodes, and finally update the file
 *            header information.
 *
 * @param[in]  page_no        the page number value of the inserted page.
 * @param[in]  hash_val       the hash value corresponding to the inserted page.
 * @param[in]  hash_val_len   the length of the hash value.
 *
 * @return     If the insertion fails, the return value is less than 0.
 */
int insertNewPageDisorder(int page_no, uchar *hash_val, uint32 hash_val_len);

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
	fd:f The file descriptor
Return:
	NULL.
*/
void deal_with_remaining_nodes_in_queue(PQNode *pQHeader, PQNode *pQ, int fd);

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

/**
 * @brief      Returning the memory address of the given section in MHT block buffer.
 *
 * @param      mht_blk_buffer      The pointer to MHT block buffer
 * @param[in]  mht_blk_buffer_len  MHT block buffer length
 * @param[in]  offset              The offset of the given section. (refering to defs.h: MHT_BLOCK_OFFSET_xxxx)
 *
 * @return     The section address in MHT block buffer.
 */
void *get_section_addr_in_mht_block_buffer(uchar *mht_blk_buffer, uint32 mht_blk_buffer_len, uint32 offset);

/**
 * @brief      Determines whether the specified offset is valid offset in MHT block buffer.
 *
 * @param[in]  offset  The given offset in MHT block buffer.
 *
 * @return     True if the specified offset is valid offset in MHT block buffer, False otherwise.
 */
bool is_valid_offset_in_mht_block_buffer(uint32 offset);

/**
 * Find the first leaf supplementary block from given offset
 * @Author   DiLu
 * @DateTime 2021-11-26T13:07:33+0800
 * @param    fd                       The file descriptor
 * @param    offset                   The given offset
 * @return                            The offset of the first leaf supplementary block, otherwise, -1 will be returned.
 */
int find_the_first_leaf_splymt_block_by_offset(int fd, int offset);

/**
 * Printing a QNode structure for debugging.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:19:35+0800
 * @param    qnode_ptr                [A pointer to a QNode structure]
 */
void print_qnode_info(PQNode qnode_ptr);


/*
计算父节点哈希值
Parameters:
	fd: 文件描述符
	parent_block_buf: 初始父节点块信息.
	offset: the 父节点对应的绝对偏移量
*/
/**
 *  @brief					  Update the MHT information according to the given node block information
 *
 * 	@param[in] 			fd:											The file descriptor.
 *	@param[in] 			parent_block_buf: 		 A pointer to the updated node block information.  
 *	@param[in] 			offset:		 						    The offset of the updated node block in the file.
 *
 *  @return					If fails,values <= 0 will be returned.
*/
void cal_parent_nodes_sha256(int fd, uchar *parent_block_buf, int offset);

/*
* 从PMHT_BLOCK生成一个队列节点
* Parameters: 
*	mhtblk_ptr:指向PMHT_BLOCK的指针
*	RMSTLPN:最右子树的叶子节点页码值
* Return: 生成的队列节点指针
 */
/**
 *  @brief					  Generate a queue node from PMHT_BLOCK
 *
 * 	@param[in] 			mhtblk_ptr:		A pointer to PMHT_BLOCK
 *	@param[in] 			RMSTLPN: 		Page no. of the leaf node of the right-most subtree. 
 *
 *  @return					A pointer to a new created queue node.
*/
PQNode makeQNodebyMHTBlock(PMHT_BLOCK mhtblk_ptr, int RMSTLPN);

/*将从文件中独到的节点信息转换成队列节点
* Parameters:
*	uchar *mht_block_buf: 读取到的信息.
*	offset:该节点信息的偏移
* Return:
*	构造好的节点指针
*/
/**
 *  @brief					  Convert the node information read from the file into a queue node
 *
 * 	@param[in] 			mht_block_buf:			The pointer to MHT block buffer
 *	@param[in] 			offset:		 						 The offset of the updated node block in the file.
 *
 *  @return					A pointer to a new created queue node.
*/
PQNode mht_buffer_to_qnode(uchar *mht_block_buf, int offset);

/*
* 利用填充节点完成插入操作时，更新由其引起的页码改变
* Parameters:
*	uchar *mht_block_buf: 读取到的信息.
*	offset:该节点信息的偏移
*	fd: 文件描述符
*/
/**
 *  @brief					  When the filling node is used to complete the insert operation, update the page number change caused by it.
 *
 * 	@param[in] 			mht_block_buf:			The pointer to MHT block buffer
 *	@param[in] 			offset:		 						 The offset of the updated node block in the file.
  *	@param[in] 			fd :                      				The file descriptor
 *
 *  @return					NULL
*/
void update_interior_nodes_pageno(uchar *mht_block_buf, int offset, int fd);

/*
* 将MHT扩充为原有的2倍。
* Parameters:
*	fd: 文件描述符
* Return: 如果成功，返回扩充后的填充节点位移量，否则返回-1.
 */
/**
 *  @brief				Expand the MHT to 2 times the original..
 *
  *	@param[in] 			fd: The file descriptor
 *
 *  @return				If successful, return the expanded padding node displacement, otherwise return -1.
*/
int extentTheMHT(int fd);

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

ssize_t fo_read_mht_file(int fd, 
                            uchar *buffer, 
                            uint32 buffer_len, 
                            int offset,         // number of bytes from whence
                            int whence);


ssize_t fo_update_mht_block(int fd, 
							uchar *buffer, 
							uint32 buffer_len, 
							int rel_distance, 
							int whence);

ssize_t fo_update_mht_block2(int fd, 
                            uchar *buffer, 
                            uint32 buffer_len, 
                            int offset,         // number of bytes from whence
                            int whence);

off_t fo_locate_mht_pos(int fd, off_t offset, int whence);

int fo_close_mhtfile(int fd);

int fo_copy_file(char* srcPath,char *destPath);

void fo_printMHTFile(int fd);

#endif