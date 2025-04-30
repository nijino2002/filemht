/**
 * @defgroup   DS ds
 *
 * @brief      This library implements dataset-related operations for evaluation purpose.
 * 			   Each dataset has the following file structure:
 *               -------------------------------------------------------------------------------------------------------
 *               | "ds_v1.0" | data length of each block (DL) | data index 1 | data 1 | data index 2 | data 2 | ...... |
 *               -------------------------------------------------------------------------------------------------------
 *               |  16 bytes |         4 bytes                |   4 bytes    |  DL    |     4 bytes  |  DL    | ...... |
 *               -------------------------------------------------------------------------------------------------------
 *
 * @author     Di Lu
 * @date       2025.1.29
 */

#ifndef	DS_H
#define DS_H

#include "defs.h"
#include "dataelem.h"
#include "mhtfile.h"

#define DS_VERSION_LEN	16
#define DS_BLOCK_SIZE_LEN  sizeof(int)
extern const char DS_VERSION[];

typedef struct _ds_header{
	char ds_ver[DS_VERSION_LEN];
	uint32 m_ds_block_size;
	uint32 m_ds_block_num;
} DS_HEADER, *PDS_HEADER;

/**
 * @brief      Creates a dataset randomly and save to the file named "filename". Indices are ordered.
 *
 * @param      filename    The newly created dataset file name.
 * @param[in]  block_size  The size of each block in the dataset (in byte).
 * @param[in]  block_num   The block number.
 *
 * @return     { 0 will be returned if success, otherwise, non-zero value will be returned. }
 */
uint32 ds_create_dataset_random(char* filename, uint32 block_size, uint32 block_num);

/**
 * @brief 					{Creates a dataset with random content and save to the file named "filename". Indices are dis-ordered.}
 * 
 * @param filename 			The newly created dataset file name.
 * @param block_size 		The size of each block in the dataset (in byte).
 * @param block_num 		The block number.
 * @return uint32 			{ 0 will be returned if success, otherwise, non-zero value will be returned. }
 */
uint32 ds_create_dataset_random_dso(char* filename, uint32 block_size, uint32 block_num);

/**
 * @brief      Creates a dataset via a given array. Indices are ordered.
 *
 * @param      filename      	The newly created dataset file name.
 * @param      array_ptr     	The array pointer.
 * @param      array_elem_size	The size of array element (in byte).
 * @param[in]  array_length  	The array length (the number of the array elements).
 *
 * @return     { 0 will be returned if success, otherwise, non-zero value will be returned. }
 */
uint32 ds_create_dataset_by_array(char* filename, void* array_ptr, uint32 array_elem_size, uint32 array_length);

/**
 * @brief      { Extending the dataset with additional "block_num" blocks which have random value.  Indices are ordered.}
 *
 * @param      filename    The original dataset file name.
 * @param[in]  block_size  The size of each block in the dataset (in byte).
 * @param[in]  block_num   The additional block number.
 *
 * @return     { The actual block number of the extended dataset will be returned if success, otherwise, 0 will be returned.  }
 */
uint32 ds_extend_dataset_random(char* filename, uint32 block_size, uint32 block_num);

/**
 * @brief      { Extending the dataset with additional "block_num" blocks which have specific character value.
 * 				 Note that the character will be duplicated block_size times as the block value.
 * 				 E.g., 'R' is selected and block_size = 8, then, the block value will be "RRRRRRRR". Indices are ordered.}
 *
 * @param      filename    The original dataset file name.
 * @param[in]  add_block_num   The additional block number.
 * @param[in]  ch          The specific character value.
 *
 * @return     { The actual block number of the extended dataset will be returned if success, otherwise, 0 will be returned. Indices are ordered.}
 */
uint32 ds_extend_dataset_with_char(char* filename, uint32 add_block_num, char ch);

/**
 * @brief      { It has the same functionality as ds_extend_dataset_with_char, but the character value is 0. Indices are ordered.}
 *
 * @param      filename    The original dataset file name.
 * @param[in]  block_num   The additional block number.
 *
 * @return     { The actual block number of the extended dataset will be returned if success, otherwise, 0 will be returned.}
 */
uint32 ds_extend_dataset_with_zero(char* filename, uint32 add_block_num);

/**
 * @brief      { Extends the dataset by a given array. Indices are ordered.}
 *
 * @param      filename      The original dataset file name.
 * @param      array_ptr     The array pointer.
 * @param[in]  array_length  The array length (the number of the array elements).
 *
 * @return     { The actual block number of the extended dataset will be returned if success, otherwise, 0 will be returned.  }
 */
uint32 ds_extend_dataset_by_array(char* filename, void* array_ptr, uint32 array_length);

/**
 * @brief      { Extends the dataset by a given dataset file. Indices are ordered.}
 *
 * @param      filename  The original dataset file name.
 * @param      input_ds  The input dataset file name.
 *
 * @return     { The actual block number of the extended dataset will be returned if success, otherwise, 0 will be returned. }
 */
uint32 ds_extend_dataset_by_ds(char* filename, char* input_ds);

/**
 * @brief      { Get the total number of blocks in the dataset. }
 *
 * @param      pds_hdr  The pointer to dataset header structure.
 *
 * @return     { The actual block number will be returned, otherwise, RETCODE_ERROR_OCCURRED will returned. }
 */
uint32 ds_get_ds_block_num(PDS_HEADER pds_hdr);

/**
 * @brief      { Get the size of each block (in byte). }
 *
 * @param      pds_hdr  The pointer to dataset header structure.
 *
 * @return     { The block size will be returned, otherwise, RETCODE_ERROR_OCCURRED will be returned. }
 */
uint32 ds_get_ds_block_size(PDS_HEADER pds_hdr);

/**
 * @brief      { Verify the dataset format. }
 *
 * @param      filename  The dataset file name.
 * @param      pds_hdr   The pointer to dataset header structure.
 *
 * @return     { TRUE will be returned if success, otherwise, FALSE will be returned.}
 */
bool ds_verify_ds(char* filename, PDS_HEADER pds_hdr);

#endif