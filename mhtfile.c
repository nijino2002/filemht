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
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF); 
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
			while ((peeked_qnode_ptr = peekQueue(g_pQHeader)) && peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
				printf("PageNo-Level: %d-%d\t", 
					popped_qnode_ptr->m_MHTNode_ptr->m_pageNo, 
					popped_qnode_ptr->m_level);
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			}
			bDequeueExec ? printf("\n\n") : nop();
			bDequeueExec = FALSE;
			bCombined = FALSE;
		}
	} // for
	//printf("%d\n", g_pQHeader->m_length);

	// processing the remaining nodes in the queue
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

/*----------  Helper Functions  ---------------*/

void deal_with_remaining_nodes_in_queue(PQNode *pQHeader, PQNode *pQ){
	PQNode qnode_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
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

	// When pQ->level > pHeader->next->m_level, loop ends.
	// Note that the loop ending condition reveals that the final root node
	// has been created.
	while((*pQ)->m_level <= (*pQHeader)->next->m_level){
		mhtnode_ptr = makeZeroMHTNode(UNASSIGNED_PAGENO);
		check_pointer(mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF);
		check_pointer(qnode_ptr, "qnode_ptr");
	}
}