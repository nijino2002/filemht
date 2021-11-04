#include "mhtfile.h"

extern PQNode g_pQHeader;
extern PQNode g_pQ;

void testMHTQueue(){
	const char str[32] = TEST_STR2;
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};
	int i = 0;
	int diff = 0;
	PQNode qnode_ptr = NULL;
	PQNode bkwd_ptr = NULL;
	PQNode current_qnode_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	PQNode peeked_qnode_ptr = NULL;
	PQNode tmp_node_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	bool bCombined = FALSE;
	bool bDequeueExec = FALSE;	// whether dequeue is executed (for printf control)

	if(g_pQHeader != NULL && g_pQ != NULL)
		freeQueue(&g_pQHeader, &g_pQ);
	else if(g_pQHeader){
		freeQueue2(&g_pQHeader);
	}
	else if(g_pQ){
		freeQueue3(&g_pQ);
	}
	else{	// both of g_pQHeader and g_pQ are NULL
		;	// do nothing
	}

	initQueue(&g_pQHeader, &g_pQ);
	check_pointer((void*)g_pQHeader, "g_pQHeader");
	check_pointer((void*)g_pQ, "g_pQ");
	for(i = 0; i < 100; i++){	// i refers to page number
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(i + 1, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		mhtnode_ptr = makeMHTNode(i+1, tmp_hash_buffer); 
		check_pointer((void*)mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, 0); 
		check_pointer((void*)qnode_ptr, "qnode_ptr");
		enqueue(&g_pQHeader, &g_pQ, qnode_ptr);

		current_qnode_ptr = g_pQ;
		while ((bkwd_ptr = lookBackward(current_qnode_ptr)) && bkwd_ptr != g_pQHeader){
			check_pointer(bkwd_ptr, "bkwd_ptr");
			if(bkwd_ptr->m_level > g_pQ->m_level)
				break;
			if(bkwd_ptr->m_level == g_pQ->m_level) {
				cbd_qnode_ptr = makeCombinedQNode(bkwd_ptr, g_pQ);
				bCombined = TRUE;
				check_pointer(cbd_qnode_ptr, "cbd_qnode_ptr");
				enqueue(&g_pQHeader, &g_pQ, cbd_qnode_ptr);
			}
			current_qnode_ptr = current_qnode_ptr->prev;
			check_pointer(current_qnode_ptr, "current_qnode_ptr");
		}

		//dequeue till encountering the new created combined node
		if(bCombined) {
			while ((peeked_qnode_ptr = peekQueue(g_pQHeader)) && 
					peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
				printf("PageNo-Level: %d-%d\t", 
					popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
					popped_qnode_ptr->m_level);
				// print_hash_value(popped_qnode_ptr->m_MHTNode_ptr->m_hash);
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			}
			bDequeueExec ? printf("\n\n") : nop();
			bDequeueExec = FALSE;
			bCombined = FALSE;
		}
	} // for
	//printf("%d\n", g_pQHeader->m_length);

	// processing the left nodes in the queue
	// Finding the first node whose level > g_pQ's level
	current_qnode_ptr = g_pQ;
	while ((bkwd_ptr = lookBackward(current_qnode_ptr)) && bkwd_ptr != g_pQHeader){
		check_pointer(bkwd_ptr, "bkwd_ptr");
		if(bkwd_ptr->m_level > g_pQ->m_level){ // node found (tmp_node_ptr)
			tmp_node_ptr = bkwd_ptr;
			break;
		}
		current_qnode_ptr = current_qnode_ptr->prev;
		check_pointer(current_qnode_ptr, "current_qnode_ptr");
	}
	// create vacant combined nodes
	diff = tmp_node_ptr->m_level - g_pQ->m_level;
	for (i = 0; i < diff; i++){
		qnode_ptr = makeCombinedQNodeFromSingleNode(g_pQ);
		//printf("%d\n", qnode_ptr->m_level);
		check_pointer(qnode_ptr, "qnode_ptr");
		enqueue(&g_pQHeader, &g_pQ, qnode_ptr);
	}

	// building the right child of root node
	cbd_qnode_ptr = makeCombinedQNode(tmp_node_ptr, g_pQ);
	check_pointer(cbd_qnode_ptr, "cbd_qnode_ptr");
	enqueue(&g_pQHeader, &g_pQ, cbd_qnode_ptr);
	// building final root node
	cbd_qnode_ptr = makeCombinedQNode(g_pQHeader->next, g_pQ);
	check_pointer(cbd_qnode_ptr, "cbd_qnode_ptr");
	enqueue(&g_pQHeader, &g_pQ, cbd_qnode_ptr);

	//dequeue all nodes
	while(popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
		printf("PageNo-Level: %d-%d\t", 
			popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
			popped_qnode_ptr->m_level);
		// free node
		deleteQNode(&popped_qnode_ptr);
	} //while

	printQueue(g_pQHeader);

	freeQueue(&g_pQHeader, &g_pQ);

	return;
}

void buildMHTFile(){
	PQNode popped_qnode_ptr = NULL;
	
	process_all_pages(&g_pQHeader, &g_pQ);
	deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ);

	//dequeue remaining nodes (actuall, only root node remains)
	while(popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
		printf("PageNo-Level: %d-%d\t", 
			popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
			popped_qnode_ptr->m_level);
		// free node
		deleteQNode(&popped_qnode_ptr);
	} //while

	printQueue(g_pQHeader);

	freeQueue(&g_pQHeader, &g_pQ);

	return;
}

/*----------  Helper Functions  ---------------*/

void process_all_pages(PQNode *pQHeader, PQNode *pQ) {
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};
	int i = 0;
	int diff = 0;
	PQNode qnode_ptr = NULL;
	PQNode bkwd_ptr = NULL;
	PQNode current_qnode_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	PQNode peeked_qnode_ptr = NULL;
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	bool bCombined = FALSE;
	bool bDequeueExec = FALSE;	// whether dequeue is executed (for printf control)

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

	initQueue(pQHeader, pQ);
	check_pointer((void*)*pQHeader, "pQHeader");
	check_pointer((void*)*pQ, "pQ");
	for(i = 0; i < 100; i++){	// i refers to page number
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(i + 1, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		mhtnode_ptr = makeMHTNode(i+1, tmp_hash_buffer); 
		check_pointer((void*)mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF); 
		check_pointer((void*)qnode_ptr, "qnode_ptr");
		enqueue(pQHeader, pQ, qnode_ptr);

		current_qnode_ptr = g_pQ;
		while ((bkwd_ptr = lookBackward(current_qnode_ptr)) && bkwd_ptr != g_pQHeader){
			check_pointer(bkwd_ptr, "bkwd_ptr");
			if(bkwd_ptr->m_level > (*pQ)->m_level)
				break;
			if(bkwd_ptr->m_level == (*pQ)->m_level) {
				lchild_ptr = bkwd_ptr;
				rchild_ptr = *pQ;
				cbd_qnode_ptr = makeCombinedQNode(lchild_ptr, rchild_ptr);
				bCombined = TRUE;
				check_pointer(cbd_qnode_ptr, "cbd_qnode_ptr");
				enqueue(pQHeader, pQ, cbd_qnode_ptr);
				deal_with_nodes_offset(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
			}
			current_qnode_ptr = current_qnode_ptr->prev;
			check_pointer(current_qnode_ptr, "current_qnode_ptr");
		}

		//dequeue till encountering the new created combined node
		if(bCombined) {
			while ((peeked_qnode_ptr = peekQueue(*pQHeader)) && peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(pQHeader, pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
				printf("PageNo|Level|LCOS|RCOS|POS: %d|%d|%d|%d|%d\t", 
					popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
					popped_qnode_ptr->m_level,
					popped_qnode_ptr->m_MHTNode_ptr->m_lchildOffset,
					popped_qnode_ptr->m_MHTNode_ptr->m_rchildOffset,
					popped_qnode_ptr->m_MHTNode_ptr->m_parentOffset);
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			}
			bDequeueExec ? printf("\n\n") : nop();
			bDequeueExec = FALSE;
			bCombined = FALSE;
		}
	} // for
}

void deal_with_remaining_nodes_in_queue(PQNode *pQHeader, PQNode *pQ){
	PQNode qnode_ptr = NULL;
	PQNode current_qnode_ptr = NULL;
	PQNode bkwd_ptr = NULL;
	PQNode cbd_qnode_ptr = NULL;
	PQNode peeked_qnode_ptr = NULL;
	PQNode popped_qnode_ptr = NULL;
	PQNode lchild_ptr = NULL;
	PQNode rchild_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	uint32 qHeaderLevel = 0;
	bool bCombined = FALSE;
	bool bDequeueExec = FALSE;	// whether dequeue is executed (for printf control)
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};

	// Both of these two pointer cannot be NULL.
	if(!*pQHeader || !*pQ){
		check_pointer(*pQHeader, "*pQHeader");
		check_pointer(*pQ, "*pQ");
		return;
	}

	// Queue cannot be empty.
	if(*pQHeader == *pQ){
		return debug_print("deal_with_remaining_nodes_in_queue", "queue cannot be empty");
	}

	/* When pQ->level > pHeader->next->m_level, loop ends.
	Note that the loop ending condition reveals that the final root node
	has been created. */

	// temporarily storing header level
	qHeaderLevel = (*pQHeader)->next->m_level;
	while((*pQ)->m_level <= qHeaderLevel){
		mhtnode_ptr = makeZeroMHTNode(UNASSIGNED_PAGENO);
		check_pointer(mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode2(mhtnode_ptr, 
							   NODELEVEL_LEAF, 
							   (uchar) TRUE,
							   (uchar) TRUE, 
							   UNASSIGNED_PAGENO);
		check_pointer(qnode_ptr, "qnode_ptr");
		enqueue(pQHeader, pQ, qnode_ptr);

		current_qnode_ptr = *pQ;
		while((bkwd_ptr = lookBackward(current_qnode_ptr)) 
			&& bkwd_ptr != *pQHeader) {
			check_pointer(bkwd_ptr, "bkwd_ptr");
			if(bkwd_ptr->m_level > (*pQ)->m_level)
				break;
			if(bkwd_ptr->m_level == (*pQ)->m_level) {
				lchild_ptr = bkwd_ptr;
				rchild_ptr = *pQ;
				cbd_qnode_ptr = makeCombinedQNode(lchild_ptr, rchild_ptr);
				bCombined = TRUE;
				check_pointer(cbd_qnode_ptr, "cbd_qnode_ptr");
				enqueue(pQHeader, pQ, cbd_qnode_ptr);
				deal_with_nodes_offset(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
			} //if
			current_qnode_ptr = current_qnode_ptr->prev;
			check_pointer(current_qnode_ptr, "current_qnode_ptr");
		} // while

		if(bCombined) {
			while((peeked_qnode_ptr = peekQueue(*pQHeader)) && 
				   peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(pQHeader, pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
				printf("PageNo|Level|LCOS|RCOS|POS: %d|%d|%d|%d|%d\t", 
					popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
					popped_qnode_ptr->m_level,
					popped_qnode_ptr->m_MHTNode_ptr->m_lchildOffset,
					popped_qnode_ptr->m_MHTNode_ptr->m_rchildOffset,
					popped_qnode_ptr->m_MHTNode_ptr->m_parentOffset);
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			} // while
			bDequeueExec ? printf("\n\n") : nop();
			bDequeueExec = FALSE;
			bCombined = FALSE;
		}// if
	} // while
}

uint32 compute_relative_distance_between_2_nodes(PQNode qnode1_ptr, 
												 PQNode qnode2_ptr,
												 uchar	pov) {
	uint32 ret_val = -1;
	uint32 counter = 0;
	PQNode tmp_node_ptr = NULL;

	if(!qnode1_ptr || !qnode2_ptr) {
		check_pointer(qnode1_ptr, "qnode1_ptr");
		check_pointer(qnode2_ptr, "qnode2_ptr");
		return ret_val;
	}

	tmp_node_ptr = qnode1_ptr;
	while(tmp_node_ptr != qnode2_ptr && tmp_node_ptr) {
		counter ++;
		tmp_node_ptr = tmp_node_ptr->next;
	}

	ret_val = pov == 0x01 ? counter : -counter;

	return ret_val;
}

void deal_with_nodes_offset(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr){
	int d1 = -1, d2 = -1;

	if(!parent_ptr || !lchild_ptr || !rchild_ptr){
		check_pointer(parent_ptr, "parent_ptr");
		check_pointer(lchild_ptr, "lchild_ptr");
		check_pointer(rchild_ptr, "rchild_ptr");
		return;
	}

	d1 = compute_relative_distance_between_2_nodes(lchild_ptr, parent_ptr, 0x02);
	d2 = compute_relative_distance_between_2_nodes(rchild_ptr, parent_ptr, 0x02);

	parent_ptr->m_MHTNode_ptr->m_lchildOffset = d1;
	parent_ptr->m_MHTNode_ptr->m_rchildOffset = d2;

	lchild_ptr->m_MHTNode_ptr->m_parentOffset = -d1;
	rchild_ptr->m_MHTNode_ptr->m_parentOffset = -d2;

	return;
}