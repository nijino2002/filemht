#include "mhtfile.h"

extern PQNode g_pQHeader;
extern PQNode g_pQ;
extern int g_mhtFileFD;

PMHT_FILE_HEADER makeMHTFileHeader(){
    PMHT_FILE_HEADER pmht_file_header = NULL;

    pmht_file_header = (PMHT_FILE_HEADER) malloc(sizeof(MHT_FILE_HEADER));

    if(!pmht_file_header){
        check_pointer(pmht_file_header, "pmht_file_header");
        debug_print("makeMHTFileHeader", "Creating MHT file header failed");
        return NULL;
    }

    memset(pmht_file_header->m_magicStr, 0, MHT_FILE_MAGIC_STRING_LEN);
    memcpy(pmht_file_header->m_magicStr, MHT_FILE_MAGIC_STRING, sizeof(MHT_FILE_MAGIC_STRING));
    pmht_file_header->m_rootNodeOffset = UNASSIGNED_OFFSET;
    pmht_file_header->m_firstSupplementaryLeafOffset = UNASSIGNED_OFFSET;
    memset(pmht_file_header->m_Reserved, RESERVED_CHAR, MHT_HEADER_RSVD_SIZE);

    return pmht_file_header;
}

void freeMHTFileHeader(PMHT_FILE_HEADER *pmht_file_header){
	(*pmht_file_header) != NULL ? free(*pmht_file_header) : nop();
	*pmht_file_header = NULL;
	return;
}

void initMHTBlock(PMHT_BLOCK *pmht_block){
	if(!*pmht_block){
		check_pointer(*pmht_block, "*pmht_block");
		debug_print("initMHTBlock", "*pmht_block cannot be NULL");
		return;
	}

	(*pmht_block)->m_pageNo = UNASSIGNED_PAGENO;
	(*pmht_block)->m_nodeLevel = 0;
	memset((*pmht_block)->m_hash, 0, HASH_LEN);
	(*pmht_block)->m_isSupplementaryNode = (uchar)FALSE;
	(*pmht_block)->m_isZeroNode = (uchar)FALSE;
	(*pmht_block)->m_lChildPageNo = UNASSIGNED_PAGENO;
	(*pmht_block)->m_lChildOffset = UNASSIGNED_OFFSET;
	(*pmht_block)->m_rChildPageNo = UNASSIGNED_PAGENO;
	(*pmht_block)->m_rChildOffset = UNASSIGNED_OFFSET;
	(*pmht_block)->m_parentPageNo = UNASSIGNED_PAGENO;
	(*pmht_block)->m_parentOffset = UNASSIGNED_OFFSET;
	memset((*pmht_block)->m_Reserved, 'R', MHT_BLOCK_RSVD_SIZE);

	return;
}

PMHT_BLOCK makeMHTBlock(){
	PMHT_BLOCK pmht_block = NULL;

	pmht_block = (PMHT_BLOCK) malloc(sizeof(MHT_BLOCK));
	if(!pmht_block)	// Failed to allocate memory
		return NULL;
	initMHTBlock(&pmht_block);

	return pmht_block;
}

void freeMHTBlock(PMHT_BLOCK *pmht_block){
	(*pmht_block) != NULL ? free(*pmht_block) : nop();
	*pmht_block = NULL;
	return;
}

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
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	uchar *mhtblk_buffer = NULL;
	uchar *mhthdr_buffer = NULL;

	// Preparing MHT file
	// Creating a new MHT file. Note that if the file exists, it will be truncated!
	if((g_mhtFileFD = fo_create_mhtfile(MHT_DEFAULT_FILE_NAME)) < 0) {
		debug_print("buildMHTFile", "Creating MHT file failed!");
		return;
	}

	// Moving file pointer to 128th bytes to 
	// reserve space for header block (root node)
	if(fo_locate_mht_pos(g_mhtFileFD, MHT_HEADER_LEN, SEEK_CUR) < 0) {
		debug_print("buildMHTFile", "Reserving space for root node failed!");
		return;
	}

	// Initializing MHT file header
	// Header will be updated at the end of building MHT file
	mht_file_header_ptr = makeMHTFileHeader();
	
	process_all_pages(&g_pQHeader, &g_pQ);
	deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ);

	//dequeue remaining nodes (actually, only root node remains)
	while(popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
		// Building MHT blocks based on dequeued nodes, then writing to MHT file.
		mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
		memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
		qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
		if(g_mhtFileFD > 0) {
			g_mhtFileRootNodeOffset = fo_locate_mht_pos(g_mhtFileFD, 0, SEEK_CUR);	//temporarily storing root node offset in MHT file
			fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);	// write root block to MHT file
		}
		free(mhtblk_buffer); mhtblk_buffer = NULL;

		print_qnode_info(popped_qnode_ptr);
		// free node
		deleteQNode(&popped_qnode_ptr);
	} //while

	/***** Updating MHT file header *****/
	mhthdr_buffer = (uchar*) malloc(MHT_HEADER_LEN);
	if(mht_file_header_ptr && mhthdr_buffer){
		mht_file_header_ptr->m_rootNodeOffset = g_mhtFileRootNodeOffset;
		mht_file_header_ptr->m_firstSupplementaryLeafOffset = g_mhtFirstSplymtLeafOffset;
		serialize_mht_file_header(mht_file_header_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
		fo_update_mht_file_header(g_mhtFileFD, mhthdr_buffer, MHT_HEADER_LEN);
		free(mhthdr_buffer);
		freeMHTFileHeader(&mht_file_header_ptr);
	}

	printQueue(g_pQHeader);

	freeQueue(&g_pQHeader, &g_pQ);
	fo_close_mhtfile(g_mhtFileFD);

	return;
}

int initOpenMHTFileWR(uchar *pathname){
	int ret = -1;

	if(!pathname){
		debug_print("initOpenMHTFile", "Null pathname");
		return ret;
	}

	// file descriptor 0, 1 and 2 refer to STDIN, STDOUT and STDERR
	if(g_mhtFileFdRd > 2) {
		fo_close_mhtfile(g_mhtFileFdRd);
	}

	// open MHT file in "READ & WRITE" mode
	g_mhtFileFdRd = fo_open_mhtfile(pathname);
	ret = g_mhtFileFdRd;

	return ret;
}

PMHT_FILE_HEADER readMHTFileHeader() {
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	uchar *mht_file_header_buffer = NULL;
	uchar *tmp_ptr = NULL;

	if(g_mhtFileFdRd < 3){
		debug_print("readMHTFileHeader", "Invalid file descriptor for reading");
		return NULL;
	}

	if(!(mht_file_header_buffer = (uchar*) malloc(MHT_HEADER_LEN))) {
		debug_print("readMHTFileHeader", "Failed to allocate memory for mht_file_header_buffer");
		return NULL;
	}
	memset(mht_file_header_buffer, 0, MHT_HEADER_LEN);

	fo_read_mht_file_header(g_mhtFileFdRd, mht_file_header_buffer, MHT_HEADER_LEN);
	// Preparing to parse MHT file header buffer into MHT file header structure
	if(!(mht_file_header_ptr = (PMHT_FILE_HEADER) malloc(sizeof(MHT_FILE_HEADER)))) {
		debug_print("readMHTFileHeader", "Failed to allocate memory for mht_file_header_ptr");
		return NULL;
	}
	tmp_ptr = mht_file_header_buffer;
	memcpy(mht_file_header_ptr->m_magicStr, tmp_ptr, MHT_FILE_MAGIC_STRING_LEN);
	tmp_ptr += MHT_FILE_MAGIC_STRING_LEN;
	memcpy(&(mht_file_header_ptr->m_rootNodeOffset), tmp_ptr, sizeof(uint32));
	tmp_ptr += sizeof(uint32);
	memcpy(&(mht_file_header_ptr->m_firstSupplementaryLeafOffset), tmp_ptr, sizeof(uint32));
	tmp_ptr += sizeof(uint32);
	memcpy(mht_file_header_ptr->m_Reserved, tmp_ptr, MHT_HEADER_RSVD_SIZE);

	return mht_file_header_ptr;
}

PMHT_BLOCK searchPageByNo(int page_no) {
	PMHT_BLOCK mhtblk_ptr = NULL;	// MHT block preserving the found page
	uchar *tmpblk_buf = NULL;	// temporarily storing MHT block buffer
	PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
	bool	bPageBlockFound = FALSE;

	if(g_mhtFileFdRd < 3){
		debug_print("searchPageByNo", "Invalid file descriptor");
		return NULL;
	}

	if(page_no < 0) {
		debug_print("searchPageByNo", "Invalid page number");
		return NULL;
	}

	if(!(mhtfilehdr_ptr = readMHTFileHeader())){
		debug_print("searchPageByNo", "Failed to read MHT file header");
		return NULL;
	}
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

	initQueue(pQHeader, pQ);
	check_pointer((void*)*pQHeader, "pQHeader");
	check_pointer((void*)*pQ, "pQ");
	for(i = 0; i < 20; i++){	// i refers to page number
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
				deal_with_interior_nodes_pageno(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
			}
			current_qnode_ptr = current_qnode_ptr->prev;
			check_pointer(current_qnode_ptr, "current_qnode_ptr");
		}

		//dequeue till encountering the new created combined node
		if(bCombined) {
			while ((peeked_qnode_ptr = peekQueue(*pQHeader)) && peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(pQHeader, pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");

				// Building MHT blocks based on dequeued nodes, then writing to MHT file.
				mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
				memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
				qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
				if(g_mhtFileFD > 0) {
					fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
				}
				free(mhtblk_buffer); mhtblk_buffer = NULL;
				/***************** CODE FOR TEST *****************/
				/*
				if(popped_qnode_ptr->m_MHTNode_ptr->m_pageNo == 1 && 
					popped_qnode_ptr->m_level == 0){
					mhtblk_buffer = (uchar*) malloc(sizeof(MHT_BLOCK));
					mht_blk_ptr = makeMHTBlock();
					convert_qnode_to_mht_block(popped_qnode_ptr, &mht_blk_ptr);
					serialize_mht_block(mht_blk_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
					print_buffer_in_byte_hex(mhtblk_buffer, MHT_BLOCK_SIZE);
					freeMHTBlock(&mht_blk_ptr);
					free(mhtblk_buffer);
				}
				*/
				/************************************************/

				print_qnode_info(popped_qnode_ptr);
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
	bool bEnctrFirstSplymtLeaf = FALSE;		// whether firstly encountering the first supplementary leaf
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};
	uchar *mhtblk_buffer = NULL;

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
				deal_with_interior_nodes_pageno(cbd_qnode_ptr, lchild_ptr, rchild_ptr);
			} //if
			current_qnode_ptr = current_qnode_ptr->prev;
			check_pointer(current_qnode_ptr, "current_qnode_ptr");
		} // while

		if(bCombined) {
			while((peeked_qnode_ptr = peekQueue(*pQHeader)) && 
				   peeked_qnode_ptr->m_level < cbd_qnode_ptr->m_level) {
				popped_qnode_ptr = dequeue(pQHeader, pQ);
				check_pointer(popped_qnode_ptr, "popped_qnode_ptr");

				// Building MHT blocks based on dequeued nodes, then writing to MHT file.
				mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
				memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
				qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
				if(g_mhtFileFD > 0) {
					if(!bEnctrFirstSplymtLeaf && 
						popped_qnode_ptr->m_MHTNode_ptr->m_pageNo == UNASSIGNED_PAGENO && 
						popped_qnode_ptr->m_level == 0) {
						g_mhtFirstSplymtLeafOffset = fo_locate_mht_pos(g_mhtFileFD, 0, SEEK_CUR);
						bEnctrFirstSplymtLeaf = TRUE;
					}
					fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
				}
				free(mhtblk_buffer); mhtblk_buffer = NULL;

				print_qnode_info(popped_qnode_ptr);
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

void deal_with_interior_nodes_pageno(PQNode parent_ptr, PQNode lchild_ptr, PQNode rchild_ptr) {
	if(!parent_ptr || !lchild_ptr || !rchild_ptr){
		check_pointer(parent_ptr, "parent_ptr");
		check_pointer(lchild_ptr, "lchild_ptr");
		check_pointer(rchild_ptr, "rchild_ptr");
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

int serialize_mht_block(PMHT_BLOCK pmht_block, 
						uchar **block_buf, 
						uint32 block_buf_len) {
	int ret = 0;	// 0 refers to none data has been processed.
	char *p_buf = NULL;

	if(!pmht_block || !*block_buf || !block_buf || block_buf_len != MHT_BLOCK_SIZE) {
		check_pointer(pmht_block, "pmht_block");
		check_pointer(*block_buf, "*block_buf");
		check_pointer(block_buf, "block_buf");
		debug_print("serialize_mht_block", "error parameters");
		return ret;
	}
	
	p_buf = *block_buf;
	memset(p_buf, 0, MHT_BLOCK_SIZE);
	memcpy(p_buf, &(pmht_block->m_pageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_nodeLevel), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, pmht_block->m_hash, HASH_LEN);
	p_buf += HASH_LEN;
	ret += HASH_LEN;
	*p_buf = pmht_block->m_isSupplementaryNode;
	p_buf += sizeof(char);
	ret += sizeof(char);
	*p_buf = pmht_block->m_isZeroNode;
	p_buf += sizeof(char);
	ret += sizeof(char);
	memcpy(p_buf, &(pmht_block->m_lChildPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_lChildOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_rChildPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_rChildOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_parentPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(pmht_block->m_parentOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, pmht_block->m_Reserved, MHT_BLOCK_RSVD_SIZE);
	ret += MHT_BLOCK_RSVD_SIZE;

	return ret;
}

int serialize_mht_file_header(PMHT_FILE_HEADER pmht_file_header, 
                            uchar **header_buf,
                            uint32 header_buf_len){
	int ret = 0;	// 0 refers to none data has been processed.
	char *p_buf = NULL;

	if(!pmht_file_header || !*header_buf || !header_buf || header_buf_len != MHT_HEADER_LEN) {
		check_pointer(pmht_file_header, "pmht_file_header");
		check_pointer(*header_buf, "*header_buf");
		check_pointer(header_buf, "header_buf");
		debug_print("serialize_mht_file_header", "error parameters");
		return ret;
	}

	p_buf = *header_buf;
	memset(p_buf, 0, MHT_HEADER_LEN);
	memcpy(p_buf, pmht_file_header->m_magicStr, MHT_FILE_MAGIC_STRING_LEN);
	p_buf += MHT_FILE_MAGIC_STRING_LEN;
	ret += MHT_FILE_MAGIC_STRING_LEN;
	memcpy(p_buf, &(pmht_file_header->m_rootNodeOffset), sizeof(uint32));
	p_buf += sizeof(uint32);
	ret += sizeof(uint32);
	memcpy(p_buf, &(pmht_file_header->m_firstSupplementaryLeafOffset), sizeof(uint32));
	p_buf += sizeof(uint32);
	ret += sizeof(uint32);
	memcpy(p_buf, pmht_file_header->m_Reserved, MHT_HEADER_RSVD_SIZE);
	ret += MHT_HEADER_RSVD_SIZE;

	return ret;
}

int unserialize_mht_block(char *block_buf, 
						  uint32 block_buf_len, 
						  PMHT_BLOCK *pmht_block) {
	/*
	pmht_block->m_pageNo = *((int*)p_buf);
	p_buf += sizeof(int);
	pmht_block->m_nodeLevel = *((int*)p_buf);
	p_buf += sizeof(int);
	memcpy(pmht_block->m_hash, p_buf, HASH_LEN);
	p_buf += HASH_LEN;
	*/
}

int qnode_to_mht_buffer(PQNode qnode_ptr, uchar **mht_block_buf, uint32 mht_block_buf_len) {
	int ret = 0;
	uchar *p_buf = NULL;

	if(!qnode_ptr || !(qnode_ptr->m_MHTNode_ptr) || !*mht_block_buf || !mht_block_buf || mht_block_buf_len != MHT_BLOCK_SIZE) {
		check_pointer(qnode_ptr, "qnode_ptr");
		check_pointer(*mht_block_buf, "*mht_block_buf");
		check_pointer(mht_block_buf, "mht_block_buf");
		debug_print("qnode_to_mht_buffer", "Error parameters");
		return ret;
	}

	memset(*mht_block_buf, 0, MHT_BLOCK_SIZE);
	p_buf = *mht_block_buf;
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_pageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_level), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, qnode_ptr->m_MHTNode_ptr->m_hash, HASH_LEN);
	p_buf += HASH_LEN;
	ret += HASH_LEN;
	*p_buf = qnode_ptr->m_is_supplementary_node;
	p_buf += sizeof(char);
	ret += sizeof(char);
	*p_buf = qnode_ptr->m_is_zero_node;
	p_buf += sizeof(char);
	ret += sizeof(char);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_lchildPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_lchildOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_rchildPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_rchildOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_parentPageNo), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memcpy(p_buf, &(qnode_ptr->m_MHTNode_ptr->m_parentOffset), sizeof(int));
	p_buf += sizeof(int);
	ret += sizeof(int);
	memset(p_buf, 'R', MHT_BLOCK_RSVD_SIZE);
	ret += MHT_BLOCK_RSVD_SIZE;

	return ret;
}

int convert_qnode_to_mht_block(PQNode qnode_ptr, PMHT_BLOCK *mhtblk_ptr) {
	if(!qnode_ptr || !(*mhtblk_ptr)){
		check_pointer(qnode_ptr, "qnode_ptr");
		check_pointer(*mhtblk_ptr, "*mhtblk_ptr");
		debug_print("convert_qnode_to_mht_block", "Null parameters");
		return -1;
	}

	(*mhtblk_ptr)->m_pageNo = qnode_ptr->m_MHTNode_ptr->m_pageNo;
	(*mhtblk_ptr)->m_nodeLevel = qnode_ptr->m_level;
	memcpy((*mhtblk_ptr)->m_hash, qnode_ptr->m_MHTNode_ptr->m_hash, HASH_LEN);
	(*mhtblk_ptr)->m_isSupplementaryNode = qnode_ptr->m_is_supplementary_node;
	(*mhtblk_ptr)->m_isZeroNode = qnode_ptr->m_is_zero_node;
	(*mhtblk_ptr)->m_lChildPageNo = qnode_ptr->m_MHTNode_ptr->m_lchildPageNo;
	(*mhtblk_ptr)->m_lChildOffset = qnode_ptr->m_MHTNode_ptr->m_lchildOffset;
	(*mhtblk_ptr)->m_rChildPageNo = qnode_ptr->m_MHTNode_ptr->m_rchildPageNo;
	(*mhtblk_ptr)->m_rChildOffset = qnode_ptr->m_MHTNode_ptr->m_rchildOffset;
	(*mhtblk_ptr)->m_parentPageNo = qnode_ptr->m_MHTNode_ptr->m_parentPageNo;
	(*mhtblk_ptr)->m_parentOffset = qnode_ptr->m_MHTNode_ptr->m_parentOffset;
	memset((*mhtblk_ptr)->m_Reserved, 'R', MHT_BLOCK_RSVD_SIZE);

	return 0;
}

void *get_section_addr_in_mht_block_buffer(uchar *mht_blk_buffer, uint32 mht_blk_buffer_len, uint32 offset) {
	if(!mht_blk_buffer || mht_blk_buffer_len != MHT_BLOCK_SIZE || !is_valid_offset_in_mht_block_buffer(offset)) {
		check_pointer(mht_blk_buffer, "mht_blk_buffer");
		debug_print("get_section_addr_in_mht_block_buffer", "Invalid parameters");
		return NULL;
	}
	
}

bool is_valid_offset_in_mht_block_buffer(uint32 offset){
	int i = 0;
	for(i = 0; i < MHT_BLOCK_ATRRIB_NUM; i++){
		if(g_MhtAttribOffsetArray[i] == offset)
			return TRUE;
	}

	return FALSE;
}

void print_qnode_info(PQNode qnode_ptr){
	if(!qnode_ptr){
		check_pointer(qnode_ptr, "qnode_ptr");
		debug_print("print_qnode_info", "Null parameters");
		return;
	}

	printf("PageNo|Level|LCPN|LCOS|RCPN|RCOS|PPN|POS: %d|%d|%d|%d|%d|%d|%d|%d\t", 
			qnode_ptr->m_MHTNode_ptr->m_pageNo, 
			qnode_ptr->m_level,
			qnode_ptr->m_MHTNode_ptr->m_lchildPageNo,
			qnode_ptr->m_MHTNode_ptr->m_lchildOffset,
			qnode_ptr->m_MHTNode_ptr->m_rchildPageNo,
			qnode_ptr->m_MHTNode_ptr->m_rchildOffset,
			qnode_ptr->m_MHTNode_ptr->m_parentPageNo,
			qnode_ptr->m_MHTNode_ptr->m_parentOffset);

	return;
}

/*----------  File Operation Functions  ------------*/

int fo_create_mhtfile(const char *pathname){
	int file_descriptor = -1;
	int open_flags;
	mode_t file_perms;

	if(!pathname){
		check_pointer((char*)pathname, "pathname");
		debug_print("fo_create_mhtfile", "Null pathname");
		return file_descriptor;
	}

	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	file_descriptor = open(pathname, open_flags, file_perms);

	return file_descriptor;
}

int fo_open_mhtfile(const char *pathname){
	int file_descriptor = -1;
	int open_flags;
	mode_t file_perms;

	if(!pathname){
		check_pointer((char*)pathname, "pathname");
		debug_print("fo_create_mhtfile", "Null pathname");
		return file_descriptor;
	}

	open_flags = O_RDWR;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	file_descriptor = open(pathname, open_flags, file_perms);

	return file_descriptor;
}

ssize_t fo_read_mht_file_header(int fd, uchar *buffer, uint32 buffer_len){
	ssize_t bytes_read = -1;

	if(fd < 0){
		debug_print("fo_read_mht_file_header", "Invalid fd");
		return bytes_read;
	}

	if(!buffer || buffer_len != MHT_HEADER_LEN){
		check_pointer(buffer, "buffer");
		debug_print("fo_read_mht_file_header", "Error parameters");
		return bytes_read;
	}

	/* Moving file pointer to the beginning of the file */
	lseek(fd, 0, SEEK_SET);
	bytes_read = read(fd, buffer, buffer_len);

	return bytes_read;
}

ssize_t fo_update_mht_file_header(int fd, uchar *buffer, uint32 buffer_len){
	ssize_t bytes_write = -1;

	if(fd < 0){
		debug_print("fo_update_mht_file_header", "Invalid fd");
		return bytes_write;
	}

	if(!buffer || buffer_len != MHT_HEADER_LEN){
		check_pointer(buffer, "buffer");
		debug_print("fo_update_mht_file_header", "Error parameters");
		return bytes_write;
	}

	/* Moving file pointer to the beginning of the file */
	lseek(fd, 0, SEEK_SET);
	bytes_write = write(fd, buffer, buffer_len);

	return bytes_write;
}

ssize_t fo_read_mht_block(int fd, 
						  uchar *buffer, 
						  uint32 buffer_len, 
						  int rel_distance, 
						  int whence){
	ssize_t bytes_read = -1;

	if(fd < 0){
		debug_print("fo_read_mht_block", "Invalid fd");
		return bytes_read;
	}

	if(!buffer || buffer_len != MHT_BLOCK_SIZE){
		check_pointer(buffer, "buffer");
		debug_print("fo_read_mht_block", "Error parameters");
		return bytes_read;
	}

	/* Moving file pointer to the beginning of the file */
	fo_locate_mht_pos(fd, rel_distance * MHT_BLOCK_SIZE, whence);
	bytes_read = read(fd, buffer, buffer_len);

	return bytes_read;
}

ssize_t fo_update_mht_block(int fd, 
							uchar *buffer, 
							uint32 buffer_len, 
							int rel_distance, 
							int whence){
	ssize_t bytes_write = -1;

	if(fd < 0){
		debug_print("fo_update_mht_block", "Invalid fd");
		return bytes_write;
	}

	if(!buffer || buffer_len != MHT_BLOCK_SIZE){
		check_pointer(buffer, "buffer");
		debug_print("fo_update_mht_block", "Error parameters");
		return bytes_write;
	}

	fo_locate_mht_pos(fd, rel_distance * MHT_BLOCK_SIZE, whence);
	bytes_write = write(fd, buffer, buffer_len);

	return bytes_write;
}

off_t fo_locate_mht_pos(int fd, off_t offset, int whence){
	if(fd < 0){
		debug_print("fo_update_mht_header_block", "Invalid fd");
		return -1;
	}

	return lseek(fd, offset, whence);
}

int fo_close_mhtfile(int fd){
	return close(fd);
}