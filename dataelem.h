#ifndef _DATAELEM_H
#define _DATAELEM_H

#include "defs.h"

/****************************************************************
 *	   BEGIN: Data  Element: Definition & Operations
*****************************************************************/

/**
 * This data structure is a general unit used to preserve data in arbitrary type in the memory.
 */
typedef	struct _DATA_ELEM {
	int 	m_index;          // data index refering to the "key"; unique identity for the data
	void 	*m_pdata;         // data in arbitrary type, which is usually allocated in the heap
	uint32	m_data_len;       // data length in byte
} DATA_ELEM, *PDATA_ELEM;

/**
 * A function type that describes the form of a "free" function 
 * used to release the resources occupied by its parameter.
 */
typedef void (*free_func)(void*);

/**
 * @brief      Initializing a DATA_ELEM object pointed by pelem
 *
 * @param[in]  pelem  The pointer pointed to a DATA_ELEM object
 */
void de_init(PDATA_ELEM pelem);

/**
 * @brief      Creating a DATA_ELEM object based on the provided data "d"
 *
 * @param[in]  idx    Data index
 * @param      d      Data provided by user
 * @param[in]  d_len  Data length
 *
 * @return     The created DATA_ELEM object
 */
PDATA_ELEM de_create(int idx, void *d, uint32 d_len);

/**
 * @brief      Free the DATA_ELEM object pointed by pelem
 *
 * @param[in]  pelem  The pointer pointing to a DATA_ELEM object
 * @param[in]  pfree  The free function used to release the resources occupied by pelem
 */
void de_free(PDATA_ELEM pelem, free_func pfree);

/****************************************************************
 *	   END: Data  Element: Definition & Operations
*****************************************************************/

#endif