#ifndef MHTFILE_EX_H
#define MHTFILE_EX_H

#include "mhtfile.h"

typedef uint32 (*extend_func)(char* file_name, uint32 d_block_size, uint32 data_block_num);

/****************************************************************
 *	   BEGIN: Data  Element: Definition & Operations
*****************************************************************/

typedef	struct _DATA_ELEM {
	int 	m_index;
	void 	*m_pdata;
	uint32	m_data_len;
} DATA_ELEM, *PDATA_ELEM;

typedef void (*free_func)(void*);

void de_init(PDATA_ELEM pelem);

PDATA_ELEM de_create(int idx, void *d, uint32 d_len);

void de_free(PDATA_ELEM pelem, free_func pfree);

/****************************************************************
 *	   END: Data  Element: Definition & Operations
*****************************************************************/

/****************************************************************
 *	       mhtfile_ex Functions
*****************************************************************/

/**
 * @brief      Building an MHT file from a given integer array.
 *
 * @param      int_array  The pointer to integer array
 * @param[in]  array_len  Array length
 */
void buildMHTFileTest_ex(int fd, PDATA_ELEM de_array, int de_array_len);

uint32 extendSupplementaryBlock4MHTFile(char* file_name, uint32 data_block_size, uint32 data_block_num, extend_func extFuncPtr);

/****************************************************************
 *	                Help Functions
*****************************************************************/

void process_all_elem(PQNode *pQHeader, 
                      PQNode *pQ, 
                      PDATA_ELEM de_array, 
                      int de_array_len);

void process_all_elem_fv(char* in_data_file,
                         char* out_mht_file,
                         PQNode *pQHeader,
                         PQNode *pQ,
                         uint32 in_data_block_size,
                         bool is_indata_hashed);

void combine_nodes_with_same_levels(PQNode *pQHeader, 
                                    PQNode *pQ, int of_fd);

void deal_with_nodes_offset_ex(PQNode parent_ptr, 
                               PQNode lchild_ptr, 
                               PQNode rchild_ptr);

uint32 compute_relative_distance_from_child_to_parent(PQNode child_ptr, 
												 	  PQNode parent_ptr,
												 	  uchar	rorl);

void deal_with_interior_nodes_pageno_ex(PQNode parent_ptr, 
                          PQNode lchild_ptr, 
                          PQNode rchild_ptr);

void update_mht_block_index_info(int of_fd, 
                                 PQNode qnode_ptr);

uint32 scan_mht_file_data_blocks(char* indata_file_name, 
                                 uint32 data_block_size);


#endif