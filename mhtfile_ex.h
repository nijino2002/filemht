/**
 * @defgroup   MHTFILE_EX mhtfile ex
 *
 * @brief     Library mhtfile_ex, including mhtfile_ex.h and mhtfile_ex.c, implements a series of extended functions 
 *            that can build an MHT file with less memory consumption compared to the implementations in the library 
 *            mhtfile (including mhtfile.h and mhtfile.c).
 *
 * @author     LU Di
 * @date       2022 Feb
 */
#ifndef MHTFILE_EX_H
#define MHTFILE_EX_H

#include "dataelem.h"
#include "mhtfile.h"

typedef uint32 (*extend_func)(char* file_name, uint32 d_block_size, uint32 data_block_num);

/****************************************************************
 *	       mhtfile_ex Functions
*****************************************************************/

/**
 * @brief      Building an MHT file from a given integer array.
 * 
 * @param      out_mht_filename  The output MHT file name
 * @param      int_array  The pointer to integer array
 * @param[in]  array_len  Array length
 */
void buildMHTFileTest_ex(const char* out_mht_filename, PDATA_ELEM de_array, int de_array_len);

/**
 * @brief      Building an MHT file from a given file
 *
 * @param      in_data_file        In data file
 * @param      out_mht_file        The out MHT file
 * @param[in]  in_data_block_size  In data block size
 * @param[in]  is_indata_hashed    Indicates if in-data is hashed
 */
void buildMHTFileFv_ex(char* in_data_file,
                         char* out_mht_file,
                         uint32 in_data_block_size,
                         bool is_indata_hashed);

/**
 * @brief      Extending an MHT file with supplementary blocks
 *
 * @param      file_name        The MHT file name
 * @param[in]  data_block_size  The supplementary data block size
 * @param[in]  data_block_num   The supplementary data block number
 * @param[in]  extFuncPtr       The function pointer pointing to a given extending function
 *
 * @return     { description_of_the_return_value }
 */
uint32 extendSupplementaryBlock4MHTFile(char* file_name, 
                                        uint32 data_block_size, 
                                        uint32 data_block_num, 
                                        extend_func extFuncPtr);

/**
 * @brief      Searching the corresponding block in MHT file by a given index.
 *
 * @param[in]  fd       The file descriptor.
 * @param[in]  index    The given index.
 *
 * @return     A new created pointer to an MHT block structure that preserving the found block.
 *             Null will be returned if errors occur or no block is found.
 */
PMHT_BLOCK searchBlockByIndex(int fd, int index);

/**
 * @brief      Locates an MHT block offset by a given index.
 *
 * @param[in]  fd       The file descriptor.
 * @param[in]  index    The given index.
 *
 * @return     If success, the offset of the block corresponding to the given index is returned,
 *             otherwise, values <= 0 will be returned.
 */
int locateMHTBlockOffsetByIndex(int fd, int index);

/****************************************************************
 *	                Help Functions
*****************************************************************/

/**
 * @brief      Building an MHT file from a given data array
 *
 * @param      out_mht_file  The output MHT file name 
 * @param      pQHeader      The queue header
 * @param      pQ            The queue tail
 * @param[in]  de_array      The data array 
 * @param[in]  de_array_len  Data array length
 */
void process_all_elem(char* out_mht_file,
                      PQNode *pQHeader, 
                      PQNode *pQ, 
                      PDATA_ELEM de_array, 
                      int de_array_len);

/**
 * @brief      Building an MHT file from a given file.
 *             Note that an in-data block consists of two parts:
 *             1) The data index (integer);
 *             2) The raw data
 *
 * @param      in_data_file        In data file name
 * @param      out_mht_file        The output MHT file name
 * @param      pQHeader            The queue header
 * @param      pQ                  The queue tail
 * @param[in]  in_data_block_size  In-data block size
 * @param[in]  is_indata_hashed    Indicates if the raw data is hashed.
 *                                 If the raw data is un-hashed, 
 *                                 it will be hashed during building the MHT file.
 */
void process_all_elem_fv(char* in_data_file,
                         char* out_mht_file,
                         PQNode *pQHeader,
                         PQNode *pQ,
                         uint32 in_data_block_size,
                         bool is_indata_hashed);

/**
 * @brief      Combining two nodes in the queue with the same levels
 *
 * @param      pQHeader  The queue header
 * @param      pQ        The queue tail
 * @param[in]  of_fd     The file descriptor of the output MHT file
 */
void combine_nodes_with_same_levels(PQNode *pQHeader, 
                                    PQNode *pQ, int of_fd);

/**
 * @brief      Dealing with the in-file offset before the node being written into the file
 *
 * @param[in]  parent_ptr  The parent pointer
 * @param[in]  lchild_ptr  The lchild pointer
 * @param[in]  rchild_ptr  The rchild pointer
 */
void deal_with_nodes_offset_ex(PQNode parent_ptr, 
                               PQNode lchild_ptr, 
                               PQNode rchild_ptr);

/**
 * @brief      Calculates the relative distance from the child to parent.
 *
 * @param[in]  child_ptr   The child pointer
 * @param[in]  parent_ptr  The parent pointer
 * @param[in]  rorl        Indicating the child is lchild ('L') or rchild ('R')
 *
 * @return     The relative distance from the child to parent.
 */
uint32 compute_relative_distance_from_child_to_parent(PQNode child_ptr, 
												 	  PQNode parent_ptr,
												 	  uchar	rorl);

/**
 * @brief      Dealing with the index of the interior nodes
 *
 * @param[in]  parent_ptr  The parent pointer
 * @param[in]  lchild_ptr  The lchild pointer
 * @param[in]  rchild_ptr  The rchild pointer
 */
void deal_with_interior_nodes_pageno_ex(PQNode parent_ptr, 
                          PQNode lchild_ptr, 
                          PQNode rchild_ptr);

/**
 * @brief      Updating the index information of the block in the MHT file 
 *             which is corresponding to the node indicated by qnode_ptr
 *
 * @param[in]  of_fd      The file descriptor of the output MHT file
 * @param[in]  qnode_ptr  The qnode pointer
 */
void update_mht_block_index_info(int of_fd, 
                                 PQNode qnode_ptr);

/**
 * @brief      Calculating the number of the data blocks in the given MHT file
 *
 * @param      indata_file_name  The input file name
 * @param[in]  data_block_size   The data block size
 *
 * @return     The number of the data blocks
 */
uint32 scan_mht_file_data_blocks(char* indata_file_name, 
                                 uint32 data_block_size);


#endif