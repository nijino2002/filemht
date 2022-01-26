#include "mhtfile_ex.h"
#include <math.h>

/****************************************************************
 * BEGIN          Data  Element: Definition & Operations
*****************************************************************/

void de_init(PDATA_ELEM pelem){
	if(!check_pointer_ex(pelem, "pelem", "de_init", "null pelem"))
		return;

	pelem->m_index = UNASSIGNED_INDEX;
	pelem->m_pdata = NULL;
	pelem->m_data_len = 0;

	return;
}

PDATA_ELEM de_create(int idx, void *d, uint32 d_len){
	PDATA_ELEM pelem = NULL;

	if(!check_pointer_ex(d, "d", "de_create", "null d"))
		return pelem;

	if(d_len <= 0){
		debug_print("de_create", "invalid d_len");
		return pelem;
	}

	pelem = (PDATA_ELEM) malloc (sizeof(DATA_ELEM));
	if(!check_pointer_ex(pelem, "pelem", "de_create", "allocating memory for pelem failed"))
		return pelem;

	de_init(pelem);

	return pelem;
}

void de_free(PDATA_ELEM pelem, free_func pfree){
	if(!pelem){
		pelem->m_pdata != NULL ? pfree(pelem->m_pdata) : nop();
		free(pelem);
		pelem = NULL;
	}

	return;
}

/****************************************************************
 * END         Data  Element: Definition & Operations
*****************************************************************/

/****************************************************************
 *                       MHT File Operations
*****************************************************************/

void buildMHTFileTest_ex(int fd, PDATA_ELEM de_array, int de_array_len){
	if(!check_pointer_ex(de_array, "de_array", "buildMHTFileTest_ex", "null de_array"))
		return;

	if(de_array_len <= 0) {
		debug_print("buildMHTFileTest_ex", "de_array_len cannot be or less than 0");
		return;
	}

	return;
}


/****************************************************************
 *                   Helper Functions
*****************************************************************/

void process_all_elem(int fd_mht, 
					  int fd_data,
					  PQNode *pQHeader, 
					  PQNode *pQ, 
					  PDATA_ELEM de_array, 
					  int de_array_len){
	const char* THIS_FUNC_NAME = "process_all_elem";
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};
	int i = 0;
	PQNode qnode_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	int out_file_fd = -1;

	if(*pQHeader != NULL && *pQ != NULL)
		freeQueue(pQHeader, pQ);
	else if(*pQHeader){
		freeQueue2(pQHeader);
	}
	else if(*pQ){
		freeQueue3(pQ);
	}
	else{	// both of g_pQHeader and g_pQ are NULL
		;	// do nothing
	}

	out_file_fd = fo_create_mhtfile("./test_out");

	// Moving file pointer to the 128th bytes to 
	// reserve space for header block
	if(fo_locate_mht_pos(out_file_fd, MHT_HEADER_LEN, SEEK_CUR) < 0) {
		debug_print(THIS_FUNC_NAME, "Reserving space for MHT file header failed!");
		return;
	}

	// Initializing MHT file header
	// Header will be updated at the end of building MHT file
	mht_file_header_ptr = makeMHTFileHeader();

	initQueue(pQHeader, pQ);
	check_pointer((void*)*pQHeader, "pQHeader");
	check_pointer((void*)*pQ, "pQ");

	for(i = 0; i < de_array_len; i++){	// i refers to page number
		combine_nodes_with_same_levels(pQHeader, pQ, out_file_fd);

		// making new node and enqueue
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(de_array[i].m_index, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		mhtnode_ptr = makeMHTNode(de_array[i].m_index, tmp_hash_buffer); 
		check_pointer_ex((void*)mhtnode_ptr, "mhtnode_ptr", THIS_FUNC_NAME, "null mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF); 
		check_pointer_ex((void*)qnode_ptr, "qnode_ptr", THIS_FUNC_NAME, "null qnode_ptr");
		enqueue(pQHeader, pQ, qnode_ptr);
	} // for

	combine_nodes_with_same_levels(pQHeader, pQ, out_file_fd);
	// dequeue the root node
	popped_qnode_ptr = dequeue(pQHeader, pQ);
	print_qnode_info(popped_qnode_ptr);
	popped_qnode_ptr->m_is_written = TRUE;
	deleteQNode(&popped_qnode_ptr);

	println();
	printQueue(*pQHeader);

	freeQueue(pQHeader, pQ);
	fo_close_mhtfile(out_file_fd);
}

void combine_nodes_with_same_levels(PQNode *pQHeader, 
                                    PQNode *pQ, int of_fd){
	const char* THIS_FUNC_NAME = "combine_nodes_with_same_levels";
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode tmp_node_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	bool bCombined = FALSE;
	uchar *mht_block_buffer = NULL;
	uint32 mht_block_buffer_len = MHT_BLOCK_SIZE;

	if(!(*pQHeader) || !(*pQ) || (*pQHeader) == (*pQ)){
		check_pointer_ex(*pQHeader, "*pQHeader", THIS_FUNC_NAME, "null *pQHeader");
		check_pointer_ex(*pQ, "*pQ", THIS_FUNC_NAME, "null *pQ");
		debug_print(THIS_FUNC_NAME, "queue is null");
		return;
	}

	mht_block_buffer = (uchar*) malloc (MHT_BLOCK_SIZE);

	while(*pQ && (*pQ)->prev && (*pQ)->prev != (*pQHeader) && (*pQ)->m_level == (*pQ)->prev->m_level){
		lchild_ptr = (*pQ)->prev;
		rchild_ptr = (*pQ);
		cbd_qnode_ptr = makeCombinedQNode(lchild_ptr, rchild_ptr);
		bCombined = TRUE;
		check_pointer_ex(cbd_qnode_ptr, "cbd_qnode_ptr", "process_all_elem", "creating cbd_qnode_ptr failed");
		enqueue(pQHeader, pQ, cbd_qnode_ptr);
		deal_with_nodes_offset_ex(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
		deal_with_interior_nodes_pageno_ex(cbd_qnode_ptr, lchild_ptr, rchild_ptr);

		tmp_node_ptr = (*pQ)->prev->prev;
		popped_qnode_ptr = dequeue_sppos(pQHeader, pQ, tmp_node_ptr);
		if(!popped_qnode_ptr->m_is_written){
			print_qnode_info(popped_qnode_ptr); println();
			memset(mht_block_buffer, 0, mht_block_buffer_len);
			qnode_to_mht_buffer(popped_qnode_ptr, &mht_block_buffer, mht_block_buffer_len);
			fo_update_mht_block2(of_fd, 
							 mht_block_buffer,
							 mht_block_buffer_len,
							 0,
							 SEEK_CUR);
			popped_qnode_ptr->m_is_written = TRUE;
			deleteQNode(&popped_qnode_ptr);
		}
		else{	// popped_qnode_ptr->m_is_written is TRUE
			// update index info. of the corresponding block in the MHT file
			if(popped_qnode_ptr->m_level > NODELEVEL_LEAF && popped_qnode_ptr->m_is_written){
				update_mht_block_index_info(of_fd, popped_qnode_ptr);
			}
		}

		tmp_node_ptr = (*pQ)->prev;
		popped_qnode_ptr = dequeue_sppos(pQHeader, pQ, tmp_node_ptr);
		if(!popped_qnode_ptr->m_is_written){
			print_qnode_info(popped_qnode_ptr); println();
			memset(mht_block_buffer, 0, mht_block_buffer_len);
			qnode_to_mht_buffer(popped_qnode_ptr, &mht_block_buffer, mht_block_buffer_len);
			fo_update_mht_block2(of_fd, 
							 mht_block_buffer,
							 mht_block_buffer_len,
							 0,
							 SEEK_CUR);
			popped_qnode_ptr->m_is_written = TRUE;
			deleteQNode(&popped_qnode_ptr);
		}
		else{	// popped_qnode_ptr->m_is_written is TRUE
			// update index info. of the corresponding block in the MHT file
			if(popped_qnode_ptr->m_level > NODELEVEL_LEAF && popped_qnode_ptr->m_is_written){
				update_mht_block_index_info(of_fd, popped_qnode_ptr);
			}
		}
		/*
		popped_qnode_ptr = dequeue_sppos(pQHeader, pQ, tmp_node_ptr);
		// update index info. of the corresponding block in the MHT file
		if(popped_qnode_ptr->m_level > NODELEVEL_LEAF && popped_qnode_ptr->m_is_written){
			update_mht_block_index_info(of_fd, popped_qnode_ptr);
		}
		!popped_qnode_ptr->m_is_written ? print_qnode_info(popped_qnode_ptr) : nop();
		println();
		popped_qnode_ptr->m_is_written = TRUE;
		memset(mht_block_buffer, 0, mht_block_buffer_len);
		qnode_to_mht_buffer(popped_qnode_ptr, &mht_block_buffer, mht_block_buffer_len);
		fo_update_mht_block2(of_fd, 
							 mht_block_buffer,
							 mht_block_buffer_len,
							 0,
							 SEEK_CUR);
		deleteQNode(&popped_qnode_ptr);
		*/

		if(!(*pQ)->m_is_written){
			print_qnode_info(*pQ); println();
			(*pQ)->m_is_written = TRUE;
			memset(mht_block_buffer, 0, mht_block_buffer_len);
			qnode_to_mht_buffer(*pQ, &mht_block_buffer, mht_block_buffer_len);
			fo_update_mht_block2(of_fd, 
							 mht_block_buffer,
							 mht_block_buffer_len,
							 0,
							 SEEK_CUR);
		}
		/*
		!(*pQ)->m_is_written ? print_qnode_info(*pQ) : nop();
		println();
		(*pQ)->m_is_written = TRUE;
		memset(mht_block_buffer, 0, mht_block_buffer_len);
		qnode_to_mht_buffer(*pQ, &mht_block_buffer, mht_block_buffer_len);
		fo_update_mht_block2(of_fd, 
							 mht_block_buffer,
							 mht_block_buffer_len,
							 0,
							 SEEK_CUR);
		*/

		printQueue(*pQHeader);
		bCombined = FALSE;
	} // while

	free(mht_block_buffer);

	return;
}

void deal_with_nodes_offset_ex(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr){
	int d1 = -1, d2 = -1;

	if(!parent_ptr || !lchild_ptr || !rchild_ptr){
		check_pointer(parent_ptr, "parent_ptr");
		check_pointer(lchild_ptr, "lchild_ptr");
		check_pointer(rchild_ptr, "rchild_ptr");
		return;
	}

	d1 = compute_relative_distance_from_child_to_parent(lchild_ptr, parent_ptr, 'L');
	d2 = compute_relative_distance_from_child_to_parent(rchild_ptr, parent_ptr, 'R');

	parent_ptr->m_MHTNode_ptr->m_lchildOffset = -d1;
	parent_ptr->m_MHTNode_ptr->m_rchildOffset = -d2;

	lchild_ptr->m_MHTNode_ptr->m_parentOffset = d1;
	rchild_ptr->m_MHTNode_ptr->m_parentOffset = d2;

	return;
}

uint32 compute_relative_distance_from_child_to_parent(PQNode child_ptr, 
												 	  PQNode parent_ptr,
												 	  uchar	rorl){
	uint32 ret_val = -1;

	if(rorl == 'R'){		// child_ptr is right child of parent_ptr
		ret_val = 1;
	}
	else if(rorl == 'L'){	// child_ptr is left child of parent_ptr
		ret_val = 2 * (int)pow(2,child_ptr->m_level) - 1 + 1;
	}
	else {
		ret_val = -1;	// invalid value
	}

	return ret_val;
}

void deal_with_interior_nodes_pageno_ex(PQNode parent_ptr, 
						  PQNode lchild_ptr, 
                          PQNode rchild_ptr){
	const char* THIS_FUNC_NAME = "deal_with_interior_nodes_pageno_ex";
	if(!parent_ptr || !lchild_ptr || !rchild_ptr){
		check_pointer_ex(parent_ptr, "parent_ptr", THIS_FUNC_NAME, "null parent_ptr");
		check_pointer_ex(lchild_ptr, "lchild_ptr", THIS_FUNC_NAME, "null lchild_ptr");
		check_pointer_ex(rchild_ptr, "rchild_ptr", THIS_FUNC_NAME, "null rchild_ptr");
		return;
	}

	/* lchild_ptr and rchild_ptr are leaf nodes. */
	if(lchild_ptr->m_level <= 0 || rchild_ptr->m_level <= 0){
		parent_ptr->m_MHTNode_ptr->m_pageNo = lchild_ptr->m_MHTNode_ptr->m_pageNo;
		parent_ptr->m_RMSTL_page_no = rchild_ptr->m_MHTNode_ptr->m_pageNo;
	}
	else{	/* lchild_ptr and rchild_ptr are interior nodes. */
		parent_ptr->m_MHTNode_ptr->m_pageNo = lchild_ptr->m_RMSTL_page_no;
		parent_ptr->m_RMSTL_page_no = rchild_ptr->m_RMSTL_page_no;
	}
	parent_ptr->m_MHTNode_ptr->m_lchildPageNo = lchild_ptr->m_MHTNode_ptr->m_pageNo;
	parent_ptr->m_MHTNode_ptr->m_rchildPageNo = rchild_ptr->m_MHTNode_ptr->m_pageNo;
	lchild_ptr->m_MHTNode_ptr->m_parentPageNo = parent_ptr->m_MHTNode_ptr->m_pageNo;
	rchild_ptr->m_MHTNode_ptr->m_parentPageNo = parent_ptr->m_MHTNode_ptr->m_pageNo;

	return;
}

void update_mht_block_index_info(int of_fd, 
                                 PQNode qnode_ptr){
	const char* THIS_FUNC_NAME = "update_mht_block_index_info";
	uchar* mht_block_buffer = NULL;
	uint32 mht_block_buffer_len = MHT_BLOCK_SIZE;
	off_t blk_offset = 0;

	if(!qnode_ptr){
		check_pointer_ex(qnode_ptr, "qnode_ptr", THIS_FUNC_NAME, "null qnode_ptr");
		return;
	}

	if(of_fd < 0){
		debug_print(THIS_FUNC_NAME, "invalid of_fd");
		return;
	}

	mht_block_buffer = (uchar*) malloc (MHT_BLOCK_SIZE);
	mht_block_buffer_len = MHT_BLOCK_SIZE;

	// now, the file pointer is at the end of the file
	if((blk_offset = fo_search_mht_block_by_qnode_info(of_fd, qnode_ptr)) >= MHT_HEADER_LEN) {
		if(blk_offset - MHT_HEADER_LEN < 0 || blk_offset - MHT_HEADER_LEN % MHT_BLOCK_SIZE != 0){
			debug_print(THIS_FUNC_NAME, "invalid blk_offset, which is not the beginning of the block");
			return;
		}
		memset(mht_block_buffer, 0, mht_block_buffer_len);
		fo_read_mht_block2(of_fd, mht_block_buffer, mht_block_buffer_len, 0, SEEK_CUR);
		*(int*)(mht_block_buffer + MHT_BLOCK_OFFSET_PAGENO) = qnode_ptr->m_MHTNode_ptr->m_pageNo;
		*(int*)(mht_block_buffer + MHT_BLOCK_OFFSET_LCPN) = qnode_ptr->m_MHTNode_ptr->m_lchildPageNo;
		*(int*)(mht_block_buffer + MHT_BLOCK_OFFSET_RCPN) = qnode_ptr->m_MHTNode_ptr->m_rchildPageNo;
		*(int*)(mht_block_buffer + MHT_BLOCK_OFFSET_PPN) = qnode_ptr->m_MHTNode_ptr->m_parentPageNo;

		// reset the file pointer to the beginning of the block
		fo_locate_mht_pos(of_fd, -MHT_BLOCK_SIZE, SEEK_CUR);
		fo_update_mht_block2(of_fd, mht_block_buffer, mht_block_buffer_len, 0, SEEK_CUR);
	}

	fo_locate_mht_pos(of_fd, 0, SEEK_END);

	free(mht_block_buffer);

	return;
}