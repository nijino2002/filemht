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
	int diff = 0;
	PQNode qnode_ptr = NULL;
	PQNode tmp_node_ptr = NULL;
	PQNode current_qnode_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	PQNode peeked_qnode_ptr = NULL;
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	uchar *counter_ary = NULL;
	bool bCombined = FALSE;
	bool bDequeueExec = FALSE;	// whether dequeue is executed (for printf control)
	PMHT_BLOCK mht_blk_ptr = NULL;
	uchar *mhtblk_buffer = NULL;

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

	counter_ary = (uchar *) malloc (sizeof(uchar) * MAX_LEVEL_COUNTER);
	memset(counter_ary, 0, MAX_LEVEL_COUNTER);

	initQueue(pQHeader, pQ);
	check_pointer((void*)*pQHeader, "pQHeader");
	check_pointer((void*)*pQ, "pQ");

/*
	for(i = 0; i < de_array_len; i++){
		printf("node %d, index: %d\n", i, de_array[i].m_index);
	}
*/

	for(i = 0; i < de_array_len; i++){	// i refers to page number
		combine_nodes_with_same_levels(pQHeader, pQ);

		// making new node and enqueue
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(de_array[i].m_index, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		mhtnode_ptr = makeMHTNode(de_array[i].m_index, tmp_hash_buffer); 
		check_pointer_ex((void*)mhtnode_ptr, "mhtnode_ptr", THIS_FUNC_NAME, "null mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF); 
		check_pointer_ex((void*)qnode_ptr, "qnode_ptr", THIS_FUNC_NAME, "null qnode_ptr");
		enqueue(pQHeader, pQ, qnode_ptr);
	} // for

	combine_nodes_with_same_levels(pQHeader, pQ);
	popped_qnode_ptr = dequeue(pQHeader, pQ);
	print_qnode_info(popped_qnode_ptr);
	deleteQNode(&popped_qnode_ptr);

	println();
	printQueue(*pQHeader);

	freeQueue(pQHeader, pQ);
	free(counter_ary);
}

void combine_nodes_with_same_levels(PQNode *pQHeader, 
                                    PQNode *pQ){
	const char* THIS_FUNC_NAME = "combine_nodes_with_same_levels";
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode tmp_node_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	bool bCombined = FALSE;

	if(!(*pQHeader) || !(*pQ) || (*pQHeader) == (*pQ)){
		check_pointer_ex(*pQHeader, "*pQHeader", THIS_FUNC_NAME, "null *pQHeader");
		check_pointer_ex(*pQ, "*pQ", THIS_FUNC_NAME, "null *pQ");
		debug_print(THIS_FUNC_NAME, "queue is null");
		return;
	}

	while(*pQ && (*pQ)->prev && (*pQ)->prev != (*pQHeader) && (*pQ)->m_level == (*pQ)->prev->m_level){
		lchild_ptr = (*pQ)->prev;
		rchild_ptr = (*pQ);
		cbd_qnode_ptr = makeCombinedQNode(lchild_ptr, rchild_ptr);
		bCombined = TRUE;
		check_pointer_ex(cbd_qnode_ptr, "cbd_qnode_ptr", "process_all_elem", "creating cbd_qnode_ptr failed");
		enqueue(pQHeader, pQ, cbd_qnode_ptr);
		deal_with_nodes_offset_ex(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
		// deal_with_interior_nodes_pageno(cbd_qnode_ptr, lchild_ptr, rchild_ptr);

		tmp_node_ptr = (*pQ)->prev->prev;
		popped_qnode_ptr = dequeue_sppos(pQHeader, pQ, tmp_node_ptr);
		!popped_qnode_ptr->m_is_written ? print_qnode_info(popped_qnode_ptr) : nop();
		println();
		popped_qnode_ptr->m_is_written = TRUE;
		deleteQNode(&popped_qnode_ptr);
		tmp_node_ptr = (*pQ)->prev;
		popped_qnode_ptr = dequeue_sppos(pQHeader, pQ, tmp_node_ptr);
		!popped_qnode_ptr->m_is_written ? print_qnode_info(popped_qnode_ptr) : nop();
		println();
		popped_qnode_ptr->m_is_written = TRUE;
		deleteQNode(&popped_qnode_ptr);
		!(*pQ)->m_is_written ? print_qnode_info(*pQ) : nop();
		println();
		(*pQ)->m_is_written = TRUE;

		printQueue(*pQHeader);
		bCombined = FALSE;
	}

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

void deal_with_node_index(PQNode qnode_ptr, 
						  uchar *lvl_ctr_ptr,
						  PDATA_ELEM de_array, 
                          int de_array_len){
	const char* THIS_FUNC_NAME = "deal_with_node_index";
	int tmp_index = 0;

	if(!qnode_ptr || !lvl_ctr_ptr){
		check_pointer_ex(qnode_ptr, "qnode_ptr", THIS_FUNC_NAME, "null qnode_ptr");
		check_pointer_ex(lvl_ctr_ptr, "lvl_ctr_ptr", THIS_FUNC_NAME, "null lvl_ctr_ptr");
		return;
	}

	if(qnode_ptr->m_level == 0)
		return;

	if(lvl_ctr_ptr[qnode_ptr->m_level] == 0) { // left child
		qnode_ptr->m_MHTNode_ptr->m_pageNo = de_array[(int)pow(2, qnode_ptr->m_level - 1)].m_index;
	}
	else if (lvl_ctr_ptr[qnode_ptr->m_level] == 1){	// right child
		qnode_ptr->m_MHTNode_ptr->m_pageNo = de_array[(int)pow(2, qnode_ptr->m_level) + (int)pow(2, qnode_ptr->m_level - 1)].m_index;
		lvl_ctr_ptr[qnode_ptr->m_level] = 0;
	}
	else{
		// invalid value
	}
	qnode_ptr->m_MHTNode_ptr->m_parentPageNo = de_array[(int)pow(2, qnode_ptr->m_level)].m_index;
	return;
}