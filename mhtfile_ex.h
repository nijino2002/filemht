#ifndef MHTFILE_EX_H
#define MHTFILE_EX_H

#include "mhtfile.h"

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


/****************************************************************
 *	                Help Functions
*****************************************************************/

void process_all_elem(int fd, PQNode *pQHeader, PQNode *pQ, PDATA_ELEM de_array, int de_array_len);

void deal_with_nodes_offset_ex(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr);

uint32 compute_relative_distance_from_child_to_parent(PQNode child_ptr, 
												 	  PQNode parent_ptr,
												 	  uchar	rorl);

#endif