#include "dataelem.h"
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
	// reserve space for header block
	if(fo_locate_mht_pos(g_mhtFileFD, MHT_HEADER_LEN, SEEK_CUR) < 0) {
		debug_print("buildMHTFile", "Reserving space for MHT file header failed!");
		return;
	}

	// Initializing MHT file header
	// Header will be updated at the end of building MHT file
	mht_file_header_ptr = makeMHTFileHeader();
	
	process_all_pages(&g_pQHeader, &g_pQ);
	/* g_pQHeader->m_length > 1 indicates that there are at least 1 floating leaf node in the queue, 
	   thus, supplementary nodes must be added to construct a complete binary tree. If g_pQHeader->m_length = 1,
	   that means only the top root node remains in the queue. In other words, the number of leaf nodes (N_l) 
	   satifies that log_2(N_l) is an integer.
	*/
	if(g_pQHeader->m_length > 1)
		deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ, g_mhtFileFD);

	//dequeue remaining nodes (actually, only root node remains)
	while(popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
		// Building MHT blocks based on dequeued nodes, then writing to MHT file.
		mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
		memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
		qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
		// set the first byte of the root buffer to 0x01, which means this buffer stores root node
		(mhtblk_buffer + MHT_BLOCK_OFFSET_RSVD)[0] = 0x01;
		if(g_mhtFileFD > 0) {
			g_mhtFileRootNodeOffset = fo_locate_mht_pos(g_mhtFileFD, 0, SEEK_CUR);	//temporarily storing root node offset in MHT file
			fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);	// write root block to MHT file
		}
		free(mhtblk_buffer); mhtblk_buffer = NULL;

#ifdef PRINT_INFO_ENABLED
		print_qnode_info(popped_qnode_ptr);
#endif
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

void buildMHTFile_fv(const char* in_file_name, const char* out_mht_file){
	const char* THIS_FUNC_NAME = "buildMHTFile_fv";
	PQNode popped_qnode_ptr = NULL;
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	uchar *mhtblk_buffer = NULL;
	uchar *mhthdr_buffer = NULL;
	int in_file_fd = -1;

	if(!check_pointer_ex((void*)in_file_name, "in_file_name", THIS_FUNC_NAME, "null in_file_name")) return;
	if(!check_pointer_ex((void*)out_mht_file, "out_mht_file", THIS_FUNC_NAME, "null out_mht_file")) return;

	in_file_fd = fo_open_mhtfile(in_file_name);
	if(in_file_fd < 0){
		debug_print(THIS_FUNC_NAME, "open in_file_name failed");
		return;
	}

	// Preparing MHT file
	// Creating a new MHT file. Note that if the file exists, it will be truncated!
	if((g_mhtFileFD = fo_create_mhtfile(out_mht_file)) < 0) {
		debug_print("buildMHTFile", "Creating MHT file failed!");
		return;
	}

	// Moving file pointer to 128th bytes to 
	// reserve space for header block
	if(fo_locate_mht_pos(g_mhtFileFD, MHT_HEADER_LEN, SEEK_CUR) < 0) {
		debug_print("buildMHTFile", "Reserving space for MHT file header failed!");
		return;
	}

	// Initializing MHT file header
	// Header will be updated at the end of building MHT file
	mht_file_header_ptr = makeMHTFileHeader();
	
	// Note that 36 means index is 4 bytes, data is 32 bytes.
	process_all_pages_fv(&g_pQHeader, &g_pQ, in_file_fd, 36, FALSE);
	/* g_pQHeader->m_length > 1 indicates that there are at least 1 floating leaf node in the queue, 
	   thus, supplementary nodes must be added to construct a complete binary tree. If g_pQHeader->m_length = 1,
	   that means only the top root node remains in the queue. In other words, the number of leaf nodes (N_l) 
	   satifies that log_2(N_l) is an integer.
	*/
	if(g_pQHeader->m_length > 1)
		deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ, g_mhtFileFD);

	//dequeue remaining nodes (actually, only root node remains)
	while(popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
		// Building MHT blocks based on dequeued nodes, then writing to MHT file.
		mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
		memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
		qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
		// set the first byte of the root buffer to 0x01, which means this buffer stores root node
		(mhtblk_buffer + MHT_BLOCK_OFFSET_RSVD)[0] = 0x01;
		if(g_mhtFileFD > 0) {
			g_mhtFileRootNodeOffset = fo_locate_mht_pos(g_mhtFileFD, 0, SEEK_CUR);	//temporarily storing root node offset in MHT file
			fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);	// write root block to MHT file
		}
		free(mhtblk_buffer); mhtblk_buffer = NULL;
#ifdef PRINT_INFO_ENABLED
		print_qnode_info(popped_qnode_ptr);
#endif
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
#ifdef PRINT_INFO_ENABLED
	printQueue(g_pQHeader);
#endif

	freeQueue(&g_pQHeader, &g_pQ);
	fo_close_mhtfile(g_mhtFileFD);
	fo_close_mhtfile(in_file_fd);

	return;
}

int initOpenMHTFileWR(char *pathname){
	if(!pathname){
		debug_print("initOpenMHTFile", "Null pathname");
		return -1;
	}

	// file descriptor 0, 1 and 2 refer to STDIN, STDOUT and STDERR
	if(g_mhtFileFdRd > 2) {
		fo_close_mhtfile(g_mhtFileFdRd);
	}

	// open MHT file in "READ & WRITE" mode
	g_mhtFileFdRd = fo_open_mhtfile(pathname);

	return g_mhtFileFdRd;
}

PMHT_FILE_HEADER readMHTFileHeader(int fd) {
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	uchar *mht_file_header_buffer = NULL;
	uchar *tmp_ptr = NULL;

	if(fd < 3){
		debug_print("readMHTFileHeader", "Invalid file descriptor for reading");
		return NULL;
	}

	if(!(mht_file_header_buffer = (uchar*) malloc(MHT_HEADER_LEN))) {
		debug_print("readMHTFileHeader", "Failed to allocate memory for mht_file_header_buffer");
		return NULL;
	}
	memset(mht_file_header_buffer, 0, MHT_HEADER_LEN);

	fo_read_mht_file_header(fd, mht_file_header_buffer, MHT_HEADER_LEN);
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

int locateMHTBlockOffsetByPageNo(int fd, int page_no) {
	PMHT_BLOCK mhtblk_ptr = NULL;	// MHT block preserving the found page
	uchar *rootnode_buf = NULL;		// root node block buffer
	uchar *tmpblk_buf = NULL;	// temporarily storing MHT block buffer
	uchar *childblk_buf = NULL;
	PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
	bool	bPageBlockFound = FALSE;
	uint32	node_level = NODELEVEL_LEAF;
	uint32	node_page_no = UNASSIGNED_PAGENO;		// temporarily preserving MHT block's page number
	int ret_offset = -1;

	if(fd < 3){
		debug_print("locateMHTBlockOffsetByPageNo", "Invalid file descriptor");
		return -1;
	}

	if(page_no < 0) {
		debug_print("locateMHTBlockOffsetByPageNo", "Invalid page number");
		return -1;
	}

	if(!(mhtfilehdr_ptr = readMHTFileHeader(fd))){
		debug_print("locateMHTBlockOffsetByPageNo", "Failed to read MHT file header");
		return -1;
	}

	rootnode_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	childblk_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(rootnode_buf, 0, MHT_BLOCK_SIZE);
	memset(childblk_buf, 0, MHT_BLOCK_SIZE);
	// after reading root node block, the file pointer is at the end of the file.
	fo_read_mht_block2(fd, rootnode_buf, MHT_BLOCK_SIZE, mhtfilehdr_ptr->m_rootNodeOffset, SEEK_SET);
	// reset the file pointer to the beginning of the root node block
	fo_locate_mht_pos(fd, -MHT_BLOCK_SIZE, SEEK_CUR);
	tmpblk_buf = rootnode_buf;
	// binary search algorithm
	while((node_level = *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_LEVEL))) > NODELEVEL_LEAF) {
		node_page_no = *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_PAGENO));
		//printf("pageNo: %d\n", node_page_no);
		if(page_no <= node_page_no){	// go to left child block
			fo_read_mht_block(fd, childblk_buf, MHT_BLOCK_SIZE, 
								*((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_LCOS)), SEEK_CUR);
		}
		else{	// go to right child block
			fo_read_mht_block(fd, childblk_buf, MHT_BLOCK_SIZE, 
								*((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_RCOS)), SEEK_CUR);
		}
		// reset the file pointer to the beginning of the current left child node block
		ret_offset = fo_locate_mht_pos(fd, -MHT_BLOCK_SIZE, SEEK_CUR);
		tmpblk_buf = childblk_buf;
	} // while

	// Now, tmpblk_buf stores the final leaf node block
	if(page_no != *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_PAGENO))) {
		//printf("offset:%d\n", ret_offset);
#ifdef PRINT_DBGINFO_ENABLED
		debug_print("locateMHTBlockOffsetByPageNo", "No page found");
#endif
		return -1;	// search failed
	}

	// free memory
	free(rootnode_buf);
	free(childblk_buf);
	free(mhtfilehdr_ptr);
	// printf("offset:%d\n", ret_offset);
	return ret_offset;
}

PMHT_BLOCK searchPageByNo(int fd, int page_no) {
	PMHT_BLOCK mhtblk_ptr = NULL;
	uchar *block_buf = NULL;

	if(locateMHTBlockOffsetByPageNo(fd, page_no) <= 0){
#ifdef PRINT_DBGINFO_ENABLED
		debug_print("searchPageByNo", "No page found");
#endif
		return NULL;
	}

	// Page block found and the file pointer is at the beginning of the block
	// read block buffer
	block_buf = (uchar*) malloc (MHT_BLOCK_SIZE);
	memset(block_buf, 0, MHT_BLOCK_SIZE);
	// fo_read_mht_block2(g_mhtFileFdRd, block_buf, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	fo_read_mht_block2(fd, block_buf, MHT_BLOCK_SIZE, 0, SEEK_CUR);

	// build a MHT block structure to store the page info.
	mhtblk_ptr = makeMHTBlock();
	unserialize_mht_block(block_buf, MHT_BLOCK_SIZE, &mhtblk_ptr);
	free(block_buf);

	return mhtblk_ptr;
	
	/*PMHT_BLOCK mhtblk_ptr = NULL;	// MHT block preserving the found page
	uchar *rootnode_buf = NULL;		// root node block buffer
	uchar *tmpblk_buf = NULL;	// temporarily storing MHT block buffer
	uchar *childblk_buf = NULL;
	PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
	bool	bPageBlockFound = FALSE;
	uint32	node_level = NODELEVEL_LEAF;
	uint32	node_page_no = UNASSIGNED_PAGENO;		// temporarily preserving MHT block's page number

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

	rootnode_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	childblk_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(rootnode_buf, 0, MHT_BLOCK_SIZE);
	memset(childblk_buf, 0, MHT_BLOCK_SIZE);
	// after reading root node block, the file pointer is at the end of the file.
	fo_read_mht_block2(g_mhtFileFdRd, rootnode_buf, MHT_BLOCK_SIZE, mhtfilehdr_ptr->m_rootNodeOffset, SEEK_SET);
	// reset the file pointer to the beginning of the root node block
	fo_locate_mht_pos(g_mhtFileFdRd, -MHT_BLOCK_SIZE, SEEK_CUR);
	tmpblk_buf = rootnode_buf;
	// binary search algorithm
	while((node_level = *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_LEVEL))) > NODELEVEL_LEAF) {
		node_page_no = *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_PAGENO));
		//printf("pageNo: %d\n", node_page_no);
		if(page_no <= node_page_no){	// go to left child block
			fo_read_mht_block(g_mhtFileFdRd, childblk_buf, MHT_BLOCK_SIZE, 
								*((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_LCOS)), SEEK_CUR);
			// reset the file pointer to the beginning of the current left child node block
			fo_locate_mht_pos(g_mhtFileFdRd, -MHT_BLOCK_SIZE, SEEK_CUR);
			tmpblk_buf = childblk_buf;
		}
		else{	// go to right child block
			fo_read_mht_block(g_mhtFileFdRd, childblk_buf, MHT_BLOCK_SIZE, 
								*((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_RCOS)), SEEK_CUR);
			// reset the file pointer to the beginning of the current left child node block
			fo_locate_mht_pos(g_mhtFileFdRd, -MHT_BLOCK_SIZE, SEEK_CUR);
			tmpblk_buf = childblk_buf;
		}
	} // while
	// final leaf node has been reached when loop ends
	if(page_no != *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_PAGENO))) {
		debug_print("searchPageByNo", "No page found");
		return NULL;
	}

	// build a MHT block structure to store the page info.
	mhtblk_ptr = makeMHTBlock();
	unserialize_mht_block(tmpblk_buf, MHT_BLOCK_SIZE, &mhtblk_ptr);
	// printf("Found page: %d\n", *((int*)(tmpblk_buf + MHT_BLOCK_OFFSET_PAGENO)));

	// free memory
	free(rootnode_buf);
	free(childblk_buf);
	free(mhtfilehdr_ptr);

	return mhtblk_ptr;
	*/
}

bool verifyHashInMHT(int fd, void* data_elem_ptr){
	const char* THIS_FUNC_NAME = "verifyHashInMHT";
	bool ret = FALSE;
	PDATA_ELEM pDE = NULL;
	PMHT_BLOCK pmht_block = NULL;
	SHA256_CTX ctx;
	char out_hash_buffer[HASH_LEN] = {0};

	if(fd < 0){
		debug_print(THIS_FUNC_NAME, "invalid file handler");
		return ret;
	}

	if(!(ret = check_pointer_ex(data_elem_ptr, "data_elem_ptr", THIS_FUNC_NAME, "Null pointer data_elem_ptr"))) {
		return ret;
	}

	pDE = (PDATA_ELEM)data_elem_ptr;
	pmht_block = searchPageByNo(fd, pDE->m_index);
	if(!pmht_block){
		printf("The MHT block with index %d cannot be found.\n", pDE->m_index);
		ret = FALSE;
		return ret;
	}

	sha256_init(&ctx);
	sha256_update(&ctx, (char*)pDE->m_pdata, pDE->m_data_len);
	sha256_final(&ctx, out_hash_buffer);

	ret = compare_two_hashes(pmht_block->m_hash, out_hash_buffer);
	freeMHTBlock(&pmht_block);

	return ret;
}

/*int updateMHTBlockHashByPageNo(int page_no, uchar *hash_val, uint32 hash_val_len) {
	uchar *block_buf = NULL;
	//???????????????MHT_block????????????
	int update_blobk_offset = 0;

	int offset = -1;

	if(page_no < 0) {
		debug_print("updateMHTBlockHashByPageNo", "Invalid page number");
		return -1;
	}

	if(!hash_val || hash_val_len != HASH_LEN){
		debug_print("updateMHTBlockHashByPageNo", "Invalid hash_val or hash_val_len");
		return -1;
	}

	if((update_blobk_offset = locateMHTBlockOffsetByPageNo(page_no)) <= 0){
		debug_print("updateMHTBlockHashByPageNo", "No page found");
		return -2;
	}

	offset = update_blobk_offset;
	//?????????????????????MHT_block???
	//??????MHT_block?????????????????????????????????
	block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(block_buf, 0, MHT_BLOCK_SIZE);
	fo_read_mht_block2(g_mhtFileFdRd, block_buf, MHT_BLOCK_SIZE, update_blobk_offset, SEEK_SET);

	//???????????????????????????????????????
	//1.??????????????????
	memcpy(block_buf + MHT_BLOCK_OFFSET_HASH, hash_val, HASH_LEN);
	//2.??????????????????????????????????????????????????????
	fo_locate_mht_pos(g_mhtFileFdRd, update_blobk_offset, SEEK_SET);
	//3.????????????
	fo_update_mht_block(g_mhtFileFdRd, block_buf, MHT_BLOCK_SIZE, 0, SEEK_CUR);

	//????????????????????????
	//???????????????????????????
	int parent_offset = 0;
	//????????????MHT????????????
	uchar *temp_block_buf = NULL;
	//??????????????????????????????
	uchar *new_hash = NULL;


	int temp_blobk_offset = 0;
	temp_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	new_hash = (uchar*) malloc(HASH_LEN);
	memset(new_hash, 0, HASH_LEN);

	printf("update_offset:%d\n", update_blobk_offset);
	while((block_buf+MHT_BLOCK_OFFSET_RSVD)[0] != 0x01 && (parent_offset = *((int*)(block_buf+MHT_BLOCK_OFFSET_POS))) !=0)
	{
		//1.?????????????????????
		temp_blobk_offset = update_blobk_offset + parent_offset * MHT_BLOCK_SIZE;
		printf("temp_blobk_offset:%d\n", temp_blobk_offset);
		memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
		fo_read_mht_block2(g_mhtFileFdRd, temp_block_buf, MHT_BLOCK_SIZE, temp_blobk_offset, SEEK_SET);

		//2.?????????????????????????????????
		cal_parent_nodes_sha256(temp_block_buf, temp_blobk_offset);
		fo_update_mht_block2(g_mhtFileFdRd, temp_block_buf, MHT_BLOCK_SIZE, temp_blobk_offset, SEEK_SET);

		//3.??????MHT???????????????
		memset(block_buf, 0, MHT_BLOCK_SIZE);
		memcpy(block_buf, temp_block_buf, MHT_BLOCK_SIZE);
		update_blobk_offset = temp_blobk_offset;
		//fo_read_mht_block2(g_mhtFileFdRd, block_buf, MHT_BLOCK_SIZE, update_blobk_offset, SEEK_SET);
		//printf("update_offset:%d\n", update_blobk_offset);
	}

	//????????????????????????????????????????????????????????????
	offset = fo_locate_mht_pos(g_mhtFileFdRd, offset, SEEK_SET);
	free(block_buf);
	free(temp_block_buf);
	free(new_hash);
	return offset;
}*/
int updateMHTBlockHashByPageNo(int page_no, uchar *hash_val, uint32 hash_val_len, int fd) {
	uchar *block_buf = NULL;
	//???????????????MHT_block????????????
	//The offset of the MHT_block that needs to be updated
	int update_blobk_offset = 0;

	int update_res = -1;

	if(page_no < 0) {
		debug_print("updateMHTBlockHashByPageNo", "Invalid page number");
		return -1;
	}

	if(!hash_val || hash_val_len != HASH_LEN){
		debug_print("updateMHTBlockHashByPageNo", "Invalid hash_val or hash_val_len");
		return -1;
	}

	if((update_blobk_offset = locateMHTBlockOffsetByPageNo(fd, page_no)) <= 0){
		debug_print("updateMHTBlockHashByPageNo", "No page found");
		return -2;
	}

	//?????????????????????MHT_block???
	//Update the MHT_block block of the specified page number
	//??????MHT_block?????????????????????????????????
	//Read MHT_block content (using absolute offset)
	block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(block_buf, 0, MHT_BLOCK_SIZE);
	fo_read_mht_block2(fd, block_buf, MHT_BLOCK_SIZE, update_blobk_offset, SEEK_SET);

	//???????????????????????????????????????
	//Write the updated hash value to the file
	//1.??????????????????
	//1. Replace the old hash value
	memcpy(block_buf + MHT_BLOCK_OFFSET_HASH, hash_val, HASH_LEN);
	//2.??????????????????????????????????????????????????????
	//2. Reposition the file read and write cursor to the beginning of the update block
	fo_locate_mht_pos(fd, update_blobk_offset, SEEK_SET);
	//3.????????????
	//3. Write to file
	fo_update_mht_block(fd, block_buf, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	//4.??????????????????
	//4. Update verification path
	update_res = updatePathToRoot(block_buf, update_blobk_offset, fd);
	//5.??????????????????????????????????????????????????????
	//5. Reposition the file read and write cursor to the beginning of the update block
	update_blobk_offset = fo_locate_mht_pos(fd, update_blobk_offset, SEEK_SET);

	free(block_buf);
	return update_res == 0 ? update_blobk_offset : 0 ;
}

int updatePathToRoot(uchar *update_block_buf, int update_blobk_offset, int fd){
	//????????????????????????
	//Update the entire verification path
	//???????????????????????????
	//Record the corresponding offset of the node
	int parent_offset = 0;
	//????????????MHT????????????
	// Temporarily store MHT node information
	uchar *temp_block_buf = NULL;
	//??????????????????????????????
	//Store the new hash value after calculation
	uchar *new_hash = NULL;

	if(!update_block_buf || update_blobk_offset <= 0){
		debug_print("updatePathToRoot", "Invalid update_block_buf or update_blobk_offset");
		return -1;
	}

	int temp_blobk_offset = 0;
	temp_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	new_hash = (uchar*) malloc(HASH_LEN);
	memset(new_hash, 0, HASH_LEN);

	temp_blobk_offset = update_blobk_offset;
	memcpy(temp_block_buf, update_block_buf, MHT_BLOCK_SIZE);
	//printf("updatePathToRoot???update_blobk_offset:%d\n", update_blobk_offset);
	while((temp_block_buf+MHT_BLOCK_OFFSET_RSVD)[0] != 0x01 && (parent_offset = *((int*)(temp_block_buf+MHT_BLOCK_OFFSET_POS))) !=0)
	{
		//1.?????????????????????
		//1. Read parent node information
		temp_blobk_offset = temp_blobk_offset + parent_offset * MHT_BLOCK_SIZE;
		//printf("updatePathToRoot???temp_blobk_offset:%d\n", temp_blobk_offset);
		memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
		fo_read_mht_block2(fd, temp_block_buf, MHT_BLOCK_SIZE, temp_blobk_offset, SEEK_SET);

		//2.?????????????????????????????????
		//2. Update its hash value and write to the file
		cal_parent_nodes_sha256(fd, temp_block_buf, temp_blobk_offset);
		fo_update_mht_block2(fd, temp_block_buf, MHT_BLOCK_SIZE, temp_blobk_offset, SEEK_SET);
	}

	free(temp_block_buf);
	free(new_hash);
	return 0;
}

int updateMHTBlockHashByMHTBlock(uchar *mhtblk_buffer, int blobk_offset, int fd){

    if(!mhtblk_buffer || blobk_offset <= 0)
    {
		debug_print("updateMHTBlockHashByMHTBlock", "Invalid mhtblk_buffer or blobk_offset");
		return -1;
	}

	//??????????????????????????????
	//Write the update information block to the file
	if( fo_update_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, blobk_offset, SEEK_SET) <= 0)
	{
		return -1;
	}

	//??????????????????
	//Update verification path
	updatePathToRoot(mhtblk_buffer, blobk_offset, fd);

    return 0;
}

int insertNewMHTBlock(PMHT_BLOCK pmht_block, int fd) {
    
    int update_blobk_offset = 0;
    //????????????????????????
	//Fill the offset of the node
    int supplementaryNode_offset = 0;
	uchar *mhtblk_buffer = NULL;
    //???????????????
	//File header information
    PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
	//?????????????????????
	//Store root node information
	uchar *rootnode_buf = NULL;
    PQNode qnode_ptr = NULL;

	//1.???????????????????????????????????????
	//1. Determine whether the pointer is empty to prevent errors
    if(!pmht_block)
    {
        debug_print("insertNewMHTBlock", "Invalid pmht_block");
        return -1;
    }

    //2.????????????????????????????????????
	//2. Find out if there are nodes that can be filled
	//2.1??????MHT????????????????????????????????????offset 
	//2.1 Obtain the offset of the filling node through the MHT file header information       
	if(fd < 3)
    {
		debug_print("insertNewMHTBlock", "Invalid file descriptor");
		return -1;
	}

	//Reset the file handler
	lseek(fd, 0, SEEK_SET);

	if(!(mhtfilehdr_ptr = readMHTFileHeader(fd)))
    {
		debug_print("insertNewMHTBlock", "Failed to read MHT file header");
		return -1;
	}
    supplementaryNode_offset = mhtfilehdr_ptr->m_firstSupplementaryLeafOffset;
#ifdef PRINT_INFO_ENABLED
	printf("supplementaryNode_offset :%d\n",supplementaryNode_offset );
#endif

	g_mhtFileRootNodeOffset = mhtfilehdr_ptr->m_rootNodeOffset;
    //2.2??????????????????????????????????????????????????????????????????;?????????????????????????????????
	//2.2 There is no node that can be filled, and a new completion node needs to be added; otherwise, proceed to the next update operation
    if(supplementaryNode_offset == 0)
    {
		supplementaryNode_offset = extentTheMHT(fd);
    }

	//3.?????????????????????
	//3. Write node information
#ifdef PRINT_INFO_ENABLED
	printf("supplementaryNode_offset: %d\n", supplementaryNode_offset);
#endif
	//?????????????????????????????????
	//Modify node information and write to file
	mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
	fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, supplementaryNode_offset, SEEK_SET);
	memcpy(mhtblk_buffer+MHT_BLOCK_OFFSET_PAGENO, &(pmht_block->m_pageNo), sizeof(int));
#ifdef PRINT_INFO_ENABLED
	printf("mhtblk_buffer + MHT_BLOCK_OFFSET_PAGENO: %d\t", *((int*)(mhtblk_buffer + MHT_BLOCK_OFFSET_PAGENO)));
#endif
	memcpy(mhtblk_buffer+MHT_BLOCK_OFFSET_HASH, pmht_block->m_hash, HASH_LEN);
	memcpy(mhtblk_buffer+MHT_BLOCK_OFFSET_ISN, &(pmht_block->m_isSupplementaryNode), sizeof(char));
#ifdef PRINT_INFO_ENABLED
	printf("pmht_block->m_isSupplementaryNode: %hhu\n", pmht_block->m_isSupplementaryNode);
#endif
	memcpy(mhtblk_buffer+MHT_BLOCK_OFFSET_IZN, &(pmht_block->m_isZeroNode), sizeof(char));
	updateMHTBlockHashByMHTBlock(mhtblk_buffer, supplementaryNode_offset, fd);

	// ??????????????????????????????
	// Modify the page number update caused by the insertion
	update_interior_nodes_pageno(mhtblk_buffer, supplementaryNode_offset, fd);
	//4.????????????????????????????????????MHT???????????????
	//4. Find the next filling node and update the MHT file header 
	supplementaryNode_offset = find_the_first_leaf_splymt_block_by_offset(fd, supplementaryNode_offset);
	//printf("update  supplementaryNode_offset: %d\n", supplementaryNode_offset);
	uchar *mhthdr_buffer = NULL;
	mhthdr_buffer = (uchar*) malloc(MHT_HEADER_LEN);
	mhtfilehdr_ptr->m_rootNodeOffset = g_mhtFileRootNodeOffset;
	mhtfilehdr_ptr->m_firstSupplementaryLeafOffset = supplementaryNode_offset == -1? 0 : supplementaryNode_offset;
	serialize_mht_file_header(mhtfilehdr_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
	fo_update_mht_file_header(fd, mhthdr_buffer, MHT_HEADER_LEN);
	
	free(mhthdr_buffer);
	free(rootnode_buf);
	free(mhtblk_buffer);
	freeMHTFileHeader(&mhtfilehdr_ptr);
    return 0;
}

int insertNewPageDisorder(int page_no, uchar *hash_val, uint32 hash_val_len, const char* mht_filename)
{
    PMHT_BLOCK mhtblk_ptr = NULL;
    uchar* read_block_buf = NULL;
    uchar* write_block_buf = NULL;
    PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
    int read_offset = UNASSIGNED_OFFSET;
    int write_offset = UNASSIGNED_OFFSET;
    int read_rootNodeOffset = UNASSIGNED_OFFSET;
	int write_rootNodeOffset = UNASSIGNED_OFFSET;
    uint32	node_level = NODELEVEL_LEAF;
	int supplementaryNode_offset = -1;
	uchar *mhthdr_buffer = NULL;

	int fd = MHT_INVALID_FILE_DSCPT;
    int new_fd = MHT_INVALID_FILE_DSCPT;

    if(page_no < 0) {
        debug_print("insertNewPageDisorder", "Invalid page number");
        return -1;
    }

    if(!hash_val || hash_val_len != HASH_LEN){
        debug_print("insertNewPageDisorder", "Invalid hash_val or hash_val_len");
        return -1;
    }

    //????????????????????????
    //Check if the page exists
	if( (fd = initOpenMHTFileWR((char*)mht_filename))  < 3){
		printf("Failed to open file %s\n", mht_filename);
		exit(0);
	}
    mhtblk_ptr = searchPageByNo(fd,page_no);
	fo_locate_mht_pos(fd, -MHT_BLOCK_SIZE, SEEK_CUR);
    read_offset = write_offset = lseek(fd, 0, SEEK_CUR);
    
    //?????????????????????????????????????????????
    //If the page already exists, perform the update operation
    if(mhtblk_ptr){
#ifdef PRINT_INFO_ENABLED
        printf("Found page: %d\n", mhtblk_ptr->m_pageNo);
#endif
        if( updateMHTBlockHashByPageNo(page_no, hash_val, HASH_LEN, fd) <= 0)
        {
            printf("Update failed.\n");
            fo_close_mhtfile(fd);
            return -1;
        }
        freeMHTBlock(&mhtblk_ptr);
        fo_close_mhtfile(fd);
        return 0;
    }
    fo_close_mhtfile(fd);
	
	fd = MHT_INVALID_FILE_DSCPT;
    //??????????????????????????????????????????
    //If the page does not exist, perform the insert operation
    //1.?????????????????????????????????????????????
    //1. Create a temporary file and copy the original file information
    fo_copy_file((char*)mht_filename, (char*)MHT_TMP_FILE_NAME);
	if( (new_fd = initOpenMHTFileWR((char*)mht_filename))  < 2){
		printf("Failed to open file %s\n", MHT_TMP_FILE_NAME);
		exit(0);
	}
	if( (fd = fo_open_mhtfile(MHT_TMP_FILE_NAME))  < 2){
		printf("Failed to open file %s\n", MHT_TMP_FILE_NAME);
		exit(0);
	}

    //2.????????????????????????
    //2. Locate the position to be inserted
    if(!(mhtfilehdr_ptr = readMHTFileHeader(new_fd)))
    {
        debug_print("insertNewPageDisorder", "Failed to read MHT file header");
        return -1;
    }
    write_rootNodeOffset = read_rootNodeOffset = mhtfilehdr_ptr->m_rootNodeOffset;
    supplementaryNode_offset = mhtfilehdr_ptr->m_firstSupplementaryLeafOffset;
#ifdef PRINT_INFO_ENABLED
    printf("rootNodeOffset :%d  supplementaryNode_offset: %d\n", read_rootNodeOffset, supplementaryNode_offset);
#endif

    ////If there is no place to insert, extend the MHT
    if(supplementaryNode_offset == UNASSIGNED_OFFSET)
    {
        supplementaryNode_offset = extentTheMHT(new_fd);
		write_rootNodeOffset = mhtfilehdr_ptr->m_rootNodeOffset = get_mhtFileRootNodeOffset();
		mhthdr_buffer = (uchar*) malloc(MHT_HEADER_LEN);
		mhtfilehdr_ptr->m_firstSupplementaryLeafOffset = supplementaryNode_offset == -1? UNASSIGNED_OFFSET : supplementaryNode_offset;
		serialize_mht_file_header(mhtfilehdr_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
		fo_update_mht_file_header(new_fd, mhthdr_buffer, MHT_HEADER_LEN);
		//printf("rootNodeOffset :%d  supplementaryNode_offset: %d\n", write_rootNodeOffset, supplementaryNode_offset);
    }

    //3.????????????
    //3. Update the file
	locateMHTBlockOffsetByPageNo(new_fd, page_no);
    read_offset = write_offset = lseek(new_fd, 0, SEEK_CUR);
    read_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
    write_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);

    //3.1 ???????????????????????????
    //3.1 Write the inserted page to the file
    //?????????????????????
    //update the information
	bool insert_isn = FALSE;
#ifdef PRINT_INFO_ENABLED
    printf("write_offset :%d   read_offset: %d\n",write_offset,read_offset);
#endif
    memset(write_block_buf, 0, MHT_BLOCK_SIZE);
    fo_read_mht_block2(new_fd, write_block_buf, MHT_BLOCK_SIZE, write_offset, SEEK_SET);
    memcpy(write_block_buf + MHT_BLOCK_OFFSET_HASH, hash_val, HASH_LEN);
    memcpy(write_block_buf + MHT_BLOCK_OFFSET_PAGENO, &page_no, sizeof(int));
	memcpy(write_block_buf + MHT_BLOCK_OFFSET_ISN, &insert_isn, sizeof(char));
    //??????????????????????????????
    //Write the update information block to the file
    if( fo_update_mht_block2(new_fd, write_block_buf, MHT_BLOCK_SIZE, write_offset, SEEK_SET) <= 0)
    {
        return -1;
    }
    //??????????????????
    //Update verification path
    updatePathToRoot(write_block_buf, write_offset, new_fd);
    //??????????????????????????????
    //Modify the page number update caused by the insertion
    update_interior_nodes_pageno(write_block_buf, write_offset, new_fd);
    write_offset += MHT_BLOCK_SIZE;
    //3.2 ???????????????????????????
    //3.2 Write the remaining pages to the file
    int no = 0;
    while(write_offset < write_rootNodeOffset && read_offset < read_rootNodeOffset)
    {
        //???????????????????????????
        //find the next write location
        memset(write_block_buf, 0, MHT_BLOCK_SIZE);
        fo_read_mht_block2(new_fd, write_block_buf, MHT_BLOCK_SIZE, write_offset, SEEK_SET);
        while((node_level = *((int*)(write_block_buf + MHT_BLOCK_OFFSET_LEVEL))) > NODELEVEL_LEAF)
        {
            memset(write_block_buf, 0, MHT_BLOCK_SIZE);
            write_offset += MHT_BLOCK_SIZE;
            if(write_offset >= write_rootNodeOffset)
            {
                break;
            }
            fo_read_mht_block2(new_fd, write_block_buf, MHT_BLOCK_SIZE, write_offset, SEEK_SET);
        }
		if(write_offset >= write_rootNodeOffset)
        {
            break;
        }
        //??????????????????????????????
        //find the leaf node to write
        memset(read_block_buf, 0, MHT_BLOCK_SIZE);
        fo_read_mht_block2(fd, read_block_buf, MHT_BLOCK_SIZE, read_offset, SEEK_SET);
        while((node_level = *((int*)(read_block_buf + MHT_BLOCK_OFFSET_LEVEL))) > NODELEVEL_LEAF)
        {
            memset(read_block_buf, 0, MHT_BLOCK_SIZE);
            read_offset += MHT_BLOCK_SIZE;
            if(read_offset >= read_rootNodeOffset)
            {
                break;
            }
            fo_read_mht_block2(fd, read_block_buf, MHT_BLOCK_SIZE, read_offset, SEEK_SET);
        }
        if(read_offset >= read_rootNodeOffset || *(read_block_buf + MHT_BLOCK_OFFSET_ISN) == TRUE)
        {
            break;
        }

        //??????????????????
        //check the write location
        //printf("no: %d,  write_offset: %d, read_offset: %d\n", no, write_offset, read_offset);
        //?????????????????????
        //update the information
        memcpy(write_block_buf + MHT_BLOCK_OFFSET_HASH, read_block_buf + MHT_BLOCK_OFFSET_HASH, HASH_LEN);
        memcpy(write_block_buf + MHT_BLOCK_OFFSET_PAGENO, read_block_buf + MHT_BLOCK_OFFSET_PAGENO, sizeof(int));
        memcpy(write_block_buf + MHT_BLOCK_OFFSET_ISN, read_block_buf + MHT_BLOCK_OFFSET_ISN, sizeof(char));

        //??????????????????????????????
        //Write the update information block to the file
        if( fo_update_mht_block2(new_fd, write_block_buf, MHT_BLOCK_SIZE, write_offset, SEEK_SET) <= 0)
        {
            return -1;
        }

        //??????????????????
        //Update verification path
        updatePathToRoot(write_block_buf, write_offset, new_fd);
        //??????????????????????????????
        //Modify the page number update caused by the insertion
        update_interior_nodes_pageno(write_block_buf, write_offset, new_fd);

        write_offset += MHT_BLOCK_SIZE;
        read_offset += MHT_BLOCK_SIZE;
        no++;
    }

    //?????????????????????
    //Update file header information
    supplementaryNode_offset = find_the_first_leaf_splymt_block_by_offset(new_fd, supplementaryNode_offset);
    mhthdr_buffer = NULL;
    mhthdr_buffer = (uchar*) malloc(MHT_HEADER_LEN);
    mhtfilehdr_ptr->m_firstSupplementaryLeafOffset = supplementaryNode_offset == -1? 0 : supplementaryNode_offset;
    serialize_mht_file_header(mhtfilehdr_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
    fo_update_mht_file_header(new_fd, mhthdr_buffer, MHT_HEADER_LEN);
#ifdef PRINT_INFO_ENABLED
	fo_printMHTFile(new_fd);
#endif

    fo_close_mhtfile(new_fd);
	fo_close_mhtfile(fd);
    freeMHTBlock(&mhtblk_ptr);
    free(read_block_buf);
    free(write_block_buf);
    free(mhtfilehdr_ptr);
    return 0;
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
	for(i = 2; i < 17; i+=4){	// i refers to page number
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

void process_all_pages_fv(PQNode *pQHeader, 
						  PQNode *pQ, 
						  int in_file_fd, 
						  uint32 in_file_data_blk_size, 
						  bool is_data_hashed) {
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
	char *in_data_read_buffer = NULL;
	int in_data_index = 0;
	uint32 in_data_bytes_read = 0;

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

	if(in_file_fd < 0){
		debug_print("process_all_pages_fv", "in_file_fd is invalid");
		return;
	}

	set_isEncounterFSLO(FALSE);

	initQueue(pQHeader, pQ);
	check_pointer((void*)*pQHeader, "pQHeader");
	check_pointer((void*)*pQ, "pQ");
	// read data block from in-data-file
	in_data_read_buffer = (char*) malloc (in_file_data_blk_size + 1);
	memset(in_data_read_buffer, 0, in_file_data_blk_size + 1);
	while((in_data_bytes_read = read(in_file_fd, in_data_read_buffer, in_file_data_blk_size)) > 0){
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		if(is_data_hashed){
			memcpy(tmp_hash_buffer, (char*)(in_data_read_buffer + sizeof(int)), SHA256_BLOCK_SIZE);
		}
		else {	// hash indata
			generateHashByBuffer_SHA256((char*)(in_data_read_buffer + sizeof(int)), in_data_bytes_read - sizeof(int), tmp_hash_buffer, SHA256_BLOCK_SIZE);
		}
		// generateHashByPageNo_SHA256(i + 1, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		memcpy(&in_data_index, (char*)in_data_read_buffer, sizeof(int));	// read index into in_data_index
		mhtnode_ptr = makeMHTNode(in_data_index, tmp_hash_buffer); 
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

				// record the offset of the first supplementary leaf node
				if(popped_qnode_ptr->m_level == NODELEVEL_LEAF && 
					popped_qnode_ptr->m_MHTNode_ptr->m_pageNo >= UNASSIGNED_INDEX){
					// mark the supplementary leaf node
					popped_qnode_ptr->m_is_supplementary_node = TRUE;
					popped_qnode_ptr->m_is_zero_node = TRUE;
					// record the offset of the first supplementary leaf node
					if(!get_isEncounterFSLO())
					{
						set_mhtFirstSplymtLeafOffset(fo_locate_mht_pos(g_mhtFileFD, 0, SEEK_CUR));
						printf("FROM process_all_pages_fv: FSOS: %d\n", get_mhtFirstSplymtLeafOffset());
						set_isEncounterFSLO(TRUE);
					}
				}

				if(g_mhtFileFD > 0) {
					fo_update_mht_block(g_mhtFileFD, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
				}
				free(mhtblk_buffer); mhtblk_buffer = NULL;
#ifdef PRINT_INFO_ENABLED
				print_qnode_info(popped_qnode_ptr);
#endif
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			}
#ifdef PRINT_INFO_ENABLED
			if(bDequeueExec){
				printf("\n\n");
			}
#endif
			bDequeueExec = FALSE;
			bCombined = FALSE;
		}// if
		memset(in_data_read_buffer, 0, in_file_data_blk_size + 1);
	}//while
	free(in_data_read_buffer);
}

void deal_with_remaining_nodes_in_queue(PQNode *pQHeader, PQNode *pQ, int fd){
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
				if(fd > 0) {
					// record the first supplementary leaf node offset to g_mhtFirstSplymtLeafOffset
					if(	popped_qnode_ptr->m_MHTNode_ptr->m_pageNo == UNASSIGNED_INDEX && 
						popped_qnode_ptr->m_level == 0)
					{
						popped_qnode_ptr->m_is_supplementary_node = TRUE;
						popped_qnode_ptr->m_is_zero_node = TRUE;
						if(!get_isEncounterFSLO()){
							set_mhtFirstSplymtLeafOffset(fo_locate_mht_pos(fd, 0, SEEK_CUR));
							printf("FROM deal_with_remaining_nodes_in_queue: FSOS: %d\n", get_mhtFirstSplymtLeafOffset());
							set_isEncounterFSLO(TRUE);
						}
					}
					/*
					if(!bEnctrFirstSplymtLeaf && 
						popped_qnode_ptr->m_MHTNode_ptr->m_pageNo == UNASSIGNED_PAGENO && 
						popped_qnode_ptr->m_level == 0) {
						g_mhtFirstSplymtLeafOffset = fo_locate_mht_pos(fd, 0, SEEK_CUR);
						bEnctrFirstSplymtLeaf = TRUE;
					}
					*/
					fo_update_mht_block(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
				}
				free(mhtblk_buffer); mhtblk_buffer = NULL;
#ifdef PRINT_INFO_ENABLED
				print_qnode_info(popped_qnode_ptr);
#endif
				deleteQNode(&popped_qnode_ptr);
				bDequeueExec = TRUE;
			} // while
#ifdef PRINT_INFO_ENABLED
			if(bDequeueExec){
				printf("\n\n");
			}
#endif
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
	int ret = 0;	// 0 refers to none data has been processed.
	PMHT_BLOCK tmpblk_ptr = NULL;

	if(!pmht_block || !block_buf || block_buf_len != MHT_BLOCK_SIZE) {
		check_pointer(pmht_block, "pmht_block");
		check_pointer(block_buf, "block_buf");
		debug_print("unserialize_mht_block", "error parameters");
		return ret;
	}

	tmpblk_ptr = *pmht_block;
	tmpblk_ptr->m_pageNo = *((int*)(block_buf + MHT_BLOCK_OFFSET_PAGENO)); ret += sizeof(int);
	tmpblk_ptr->m_nodeLevel = *((int*)(block_buf + MHT_BLOCK_OFFSET_LEVEL)); ret += sizeof(int);
	memcpy(tmpblk_ptr->m_hash, block_buf + MHT_BLOCK_OFFSET_HASH, HASH_LEN); ret += HASH_LEN;
	tmpblk_ptr->m_isSupplementaryNode = *((char*)(block_buf + MHT_BLOCK_OFFSET_ISN)); ret += sizeof(char);
	tmpblk_ptr->m_isZeroNode = *((char*)(block_buf + MHT_BLOCK_OFFSET_IZN)); ret += sizeof(char);
	tmpblk_ptr->m_lChildPageNo = *((int*)(block_buf + MHT_BLOCK_OFFSET_LCPN)); ret += sizeof(int);
	tmpblk_ptr->m_lChildOffset = *((int*)(block_buf + MHT_BLOCK_OFFSET_LCOS)); ret += sizeof(int);
	tmpblk_ptr->m_rChildPageNo = *((int*)(block_buf + MHT_BLOCK_OFFSET_RCPN)); ret += sizeof(int);
	tmpblk_ptr->m_rChildOffset = *((int*)(block_buf + MHT_BLOCK_OFFSET_RCOS)); ret += sizeof(int);
	tmpblk_ptr->m_parentPageNo = *((int*)(block_buf + MHT_BLOCK_OFFSET_PPN)); ret += sizeof(int);
	tmpblk_ptr->m_parentOffset = *((int*)(block_buf + MHT_BLOCK_OFFSET_POS)); ret += sizeof(int);
	memcpy(tmpblk_ptr->m_Reserved, block_buf + MHT_BLOCK_OFFSET_RSVD, MHT_BLOCK_RSVD_SIZE); ret += MHT_BLOCK_RSVD_SIZE;

	return ret;
}

int unserialize_mht_file_header(char *header_buf, 
								uint32 header_buf_len, 
								PMHT_FILE_HEADER *pmht_header){
	const char* THIS_FUNC_NAME = "unserialize_mht_file_header";
	int ret = 0;
	PMHT_FILE_HEADER tmp_hdr_ptr = NULL;
	char* buf_ptr = NULL;

	if(!check_pointer_ex(header_buf, "header_buf", THIS_FUNC_NAME, "Null header_buf") || 
	   !check_pointer_ex(*pmht_header, "pmht_header", THIS_FUNC_NAME, "Null header pointer for output"))
		return ret;

	if(header_buf_len != MHT_HEADER_LEN){
		debug_print(THIS_FUNC_NAME, "Invalid header length");
		return ret;
	}

	tmp_hdr_ptr = *pmht_header;
	buf_ptr = header_buf;
	memcpy(tmp_hdr_ptr->m_magicStr, buf_ptr, MHT_FILE_MAGIC_STRING_LEN); ret += MHT_FILE_MAGIC_STRING_LEN;
	buf_ptr += MHT_FILE_MAGIC_STRING_LEN;
	tmp_hdr_ptr->m_rootNodeOffset = *(int*)buf_ptr; ret += sizeof(uint32);
	buf_ptr += sizeof(uint32);
	tmp_hdr_ptr->m_firstSupplementaryLeafOffset = *(int*)buf_ptr; ret += sizeof(uint32);
	buf_ptr += sizeof(uint32);
	memcpy(tmp_hdr_ptr->m_Reserved, buf_ptr, MHT_HEADER_RSVD_SIZE); ret += MHT_HEADER_RSVD_SIZE;

	return ret;
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
	
	return (void*)(mht_blk_buffer + offset);
}

int get_block_num_in_mhtfile_by_fd(int mht_fd, int flag){
	const char* THIS_FUNC_NAME = "get_block_num_in_mhtfile_by_fd";
	int count = 0;
	int file_size = 0;
	char* read_buffer = NULL;
	int bytes_read = 0;

	if(mht_fd < 3) {
		debug_print(THIS_FUNC_NAME, "invalid file handler");
		return 0;
	}

	if(flag !=0 && flag != 1){
		debug_print(THIS_FUNC_NAME, "invalid flag value");
		return 0;
	}

	read_buffer = (char*) malloc (MHT_BLOCK_SIZE);
	memset(read_buffer, 0, MHT_BLOCK_SIZE);
	file_size = fo_locate_mht_pos(mht_fd, 0, SEEK_END);
	fo_locate_mht_pos(mht_fd, MHT_HEADER_LEN, SEEK_SET);
	while(bytes_read = fo_read_mht_file(mht_fd, read_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR) > 0){
		if(flag == NODELEVEL_LEAF && 
			*((int*)get_section_addr_in_mht_block_buffer(read_buffer, MHT_BLOCK_SIZE, MHT_BLOCK_OFFSET_LEVEL)) == NODELEVEL_LEAF){
			count ++;
		}
		else if(flag == 1)
			count ++;
		else
			nop();
		
		memset(read_buffer, 0, MHT_BLOCK_SIZE);
	}

	free(read_buffer);
	
	return count;
}

int get_block_num_in_mhtfile_by_filename(char* mht_filename, int flag){
	const char* THIS_FUNC_NAME = "get_block_num_in_mhtfile_by_filename";
	int fd = -1;
	int count = 0;

	fd = fo_open_mhtfile(mht_filename);
	if(fd < 3) {
		debug_print(THIS_FUNC_NAME, "Failed to open file");
		return 0;
	}

	count = get_block_num_in_mhtfile_by_fd(fd, flag);
	fo_close_mhtfile(fd);

	return count;
}

bool is_valid_offset_in_mht_block_buffer(uint32 offset){
	int i = 0;
	for(i = 0; i < MHT_BLOCK_ATRRIB_NUM; i++){
		if(g_MhtAttribOffsetArray[i] == offset)
			return TRUE;
	}

	return FALSE;
}

int find_the_first_leaf_splymt_block_by_offset(int fd, int offset) {
	uchar node_type = 'R';
	int reserved_val = 'RRRR';
	uchar reserved_buffer[MHT_BLOCK_RSVD_SIZE] = {0};
	uchar *mht_buf_ptr = NULL;

	if(fd < 3) {
		debug_print("find_the_first_leaf_splymt_block_by_offset", "Invalid file descriptor fd");
		return -1;
	}

	if(offset < 0) {
		debug_print("find_the_first_leaf_splymt_block_by_offset", "offset must >= 0");
		return -1;
	}

	// force the file pointer to offset
	fo_locate_mht_pos(fd, offset, SEEK_SET);
	// offset must be the beginning of an MHT block
	fo_read_mht_file(fd, reserved_buffer, MHT_BLOCK_RSVD_SIZE, -MHT_BLOCK_RSVD_SIZE, SEEK_CUR);
	if(reserved_val != *((int*)reserved_buffer)) {
		debug_print("find_the_first_leaf_splymt_block_by_offset", "offset must be the beginning of an MHT block");
		return -1;
	}
	// now, the file pointer is still at the given offset
	// read MHT block
	mht_buf_ptr = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(mht_buf_ptr, 0, MHT_BLOCK_SIZE);
	fo_read_mht_block(fd, mht_buf_ptr, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	// check whether reaching at the root block
	while((mht_buf_ptr + MHT_BLOCK_OFFSET_RSVD)[0] != 0x01) {
		// proper block is found
		if(*((int*)(mht_buf_ptr + MHT_BLOCK_OFFSET_LEVEL)) == NODELEVEL_LEAF && 
			*(mht_buf_ptr + MHT_BLOCK_OFFSET_ISN) == TRUE) {
				//printf("FFL:%hhu\n", *(mht_buf_ptr + MHT_BLOCK_OFFSET_ISN));
			return fo_locate_mht_pos(fd, 0, SEEK_CUR) - MHT_BLOCK_SIZE;
		}
		memset(mht_buf_ptr, 0, MHT_BLOCK_SIZE);
		fo_read_mht_block(fd, mht_buf_ptr, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	}

	free(mht_buf_ptr);
	return -1;	// no proper block is found
}

void print_mht_block(uchar *mht_block_buf, uint32 mht_blk_buffer_len){
	const char* THIS_FUNC_NAME = "print_mht_block";
	char *out_str = NULL;
	int out_str_len = HASH_LEN * 2 + 1;

	if(!mht_block_buf){
		check_pointer_ex(mht_block_buf, "mht_block_buf", THIS_FUNC_NAME, "null mht_block_buf");
		return;
	}

	if(mht_blk_buffer_len <= 0){
		debug_print(THIS_FUNC_NAME, "invalid mht_blk_buffer_len");
		return;
	}

	out_str = (char*) malloc (out_str_len);
	memset(out_str, 0, out_str_len);
	convert_hash_to_string((BYTE*)(mht_block_buf + MHT_BLOCK_OFFSET_HASH), out_str, out_str_len);

	printf("=========================================================================================================================\n");
	printf("|PN\t|NL\t|HASH\t\t\t\t\t\t\t\t|ISN\t|IZN\t|LCPN\t|LCOS\t|RCPN\t|RCOS\t|PPN\t|POS\t|RSVD|\n");
	printf("|-----------------------------------------------------------------------------------------------------------------------|\n");
	printf("|");
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_PAGENO));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_LEVEL));
	printf("%s|", out_str); free(out_str);
	printf("%d\t|", *(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_ISN));
	printf("%d\t|", *(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_IZN));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_LCPN));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_LCOS));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_RCPN));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_RCOS));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_PPN));
	printf("%d\t|", *(int*)(mht_block_buf + MHT_BLOCK_OFFSET_POS));
	printf("\t%c%c%c%c|\n", 
		*(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_RSVD),
		*(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_RSVD + 1),
		*(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_RSVD + 2),
		*(uchar*)(mht_block_buf + MHT_BLOCK_OFFSET_RSVD + 3)
	);
	printf("=========================================================================================================================\n");

	return;
}

void cal_parent_nodes_sha256(int fd, uchar *parent_block_buf, int offset)
{
	//??????????????????????????????
	//Store left and right child node information
	int lchild_offset = 0;
	int rchild_offset = 0;
	uchar *lhash = NULL;
	uchar *rhash = NULL;
	//?????????????????????????????????
	//Store the calculated new hash value
	uchar *new_hash = NULL;

	//??????????????????
	//Get corresponding information
	check_pointer((void*)parent_block_buf, "update_parent_block_buf");
	lchild_offset = *((int*)(parent_block_buf + MHT_BLOCK_OFFSET_LCOS));
	rchild_offset = *((int*)(parent_block_buf + MHT_BLOCK_OFFSET_RCOS));

	lhash = (uchar*) malloc(HASH_LEN);
	rhash = (uchar*) malloc(HASH_LEN);
	memset(lhash, 0, HASH_LEN);
	memset(rhash, 0, HASH_LEN);
	fo_read_mht_file(fd, lhash, HASH_LEN, lchild_offset*MHT_BLOCK_SIZE+MHT_BLOCK_OFFSET_HASH+offset, SEEK_SET);
	fo_read_mht_file(fd, rhash, HASH_LEN, rchild_offset*MHT_BLOCK_SIZE+MHT_BLOCK_OFFSET_HASH+offset, SEEK_SET);

	//??????????????????????????????
	//Calculate the new hash value and replace
	new_hash = (uchar*) malloc(HASH_LEN);
	memset(new_hash, 0, HASH_LEN);
	generateCombinedHash_SHA256(lhash, rhash, new_hash, HASH_LEN);
	memcpy(parent_block_buf + MHT_BLOCK_OFFSET_HASH, new_hash, HASH_LEN);
	//?????????????????????????????????
	//Test and output the newly obtained hash value
	//uchar hash_string[HASH_STR_LEN];
	//memset(hash_string, 0, HASH_STR_LEN);
	//convert_hash_to_string(parent_block_buf + MHT_BLOCK_OFFSET_HASH, hash_string, HASH_STR_LEN);
	//printf("The cal hash value: %s\n", hash_string);

	free(lhash);
	free(rhash);
	free(new_hash);	
}

PQNode makeQNodebyMHTBlock(PMHT_BLOCK mhtblk_ptr, int RMSTLPN)
{
	//printf("test makeQNodebyMHTBlock begin\n");
	PQNode node_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;

	if(!mhtblk_ptr)
		return NULL;

	node_ptr = (PQNode) malloc(sizeof(QNode));
	if(node_ptr == NULL)
		return NULL;

	mhtnode_ptr = (PMHTNode)malloc(sizeof(MHTNode));
	if(mhtnode_ptr == NULL)
		return NULL;
	mhtnode_ptr->m_pageNo = mhtblk_ptr->m_pageNo;
	memcpy(mhtnode_ptr->m_hash, mhtblk_ptr->m_hash, HASH_LEN);
	mhtnode_ptr->m_lchildOffset = mhtblk_ptr->m_lChildOffset;
	mhtnode_ptr->m_rchildOffset = mhtblk_ptr->m_rChildOffset;
	mhtnode_ptr->m_parentOffset = mhtblk_ptr->m_parentOffset;
	mhtnode_ptr->m_lchildPageNo = mhtblk_ptr->m_lChildPageNo;
	mhtnode_ptr->m_rchildPageNo = mhtblk_ptr->m_rChildPageNo;
	mhtnode_ptr->m_parentPageNo = mhtblk_ptr->m_parentPageNo;


	node_ptr->m_level = mhtblk_ptr->m_nodeLevel;
	node_ptr->m_MHTNode_ptr = mhtnode_ptr;
	node_ptr->m_is_supplementary_node = mhtblk_ptr->m_isSupplementaryNode;
	node_ptr->m_is_zero_node = mhtblk_ptr->m_isZeroNode;
	node_ptr->m_RMSTL_page_no = RMSTLPN;
	node_ptr->prev = NULL;
	node_ptr->next = NULL;

	return node_ptr;
}

PQNode mht_buffer_to_qnode(uchar *mht_block_buf, int offset, int mht_fd)
{
	//printf("test mht_buffer_to_qnode begin\n");
	PQNode qnode_ptr = NULL;
	int block_offset = 0;
	int rchild_offset = 0;
	//????????????MHT????????????
	// Temporarily store MHT node information
	uchar *temp_block_buf = NULL;
	uint32 most_right_pgno = UNASSIGNED_PAGENO;
	PMHT_BLOCK mhtblk_ptr = NULL;

	if(!mht_block_buf)
	{
		debug_print("mht_buffer_to_qnode", "Invalid mht_block_buf");
		return NULL;
	}
	//memcpy(mht_block_buf+MHT_BLOCK_OFFSET_RSVD,)
	memset(mht_block_buf+MHT_BLOCK_OFFSET_RSVD, 'R', MHT_BLOCK_RSVD_SIZE);
	//??????????????????????????????
	//Find the page number of its rightmost subtree
	block_offset = offset;
	temp_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
	memcpy(temp_block_buf, mht_block_buf, MHT_BLOCK_SIZE);
	while((rchild_offset = *((int*)(temp_block_buf + MHT_BLOCK_OFFSET_RCOS))) !=0)
	{
		//???????????????????????????
		//Read the right child node information
		block_offset = block_offset + rchild_offset * MHT_BLOCK_SIZE;
		//printf("block_offset:%d\n", block_offset);
		memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
		fo_read_mht_block2(mht_fd, temp_block_buf, MHT_BLOCK_SIZE, block_offset, SEEK_SET);
	}
	most_right_pgno = *((int*)(temp_block_buf + MHT_BLOCK_OFFSET_PAGENO));
	//printf("most_right_pgno:%d\n", most_right_pgno);
	
	//??????????????????
	//Construct the queue node
	//1.??????MHT??????
	//1. Restore the MHT node
	mhtblk_ptr = makeMHTBlock();
	unserialize_mht_block(mht_block_buf, MHT_BLOCK_SIZE, &mhtblk_ptr);
	//2.??????????????????
	//2. Generate queue node
	qnode_ptr = makeQNodebyMHTBlock(mhtblk_ptr, most_right_pgno);
	if(!qnode_ptr)
	{
		return NULL;
	}
	return qnode_ptr;
}

void update_interior_nodes_pageno(uchar *mht_block_buf, int offset, int fd)
{
	uchar *pmht_buf_ptr = NULL;
	uchar *lmht_buf_ptr = NULL;
	uchar *rmht_buf_ptr = NULL;
	int parent_offset = UNASSIGNED_OFFSET;
	int lchild_offset = UNASSIGNED_OFFSET;
	int rchild_offset = UNASSIGNED_OFFSET;
	uint32 rmsl_pgno = UNASSIGNED_PAGENO;
	uint32 parent_pgno = UNASSIGNED_PAGENO;
	uint32 lchild_pgno = UNASSIGNED_PAGENO;
	uint32 rchild_pgno = UNASSIGNED_PAGENO;

	int temp_offset = UNASSIGNED_OFFSET;

	if(!mht_block_buf){
		debug_print("mht_buffer_to_qnode", "Invalid mht_block_buf");
		return ;
	}

	if(fd < 3)
    {
		debug_print("update_interior_nodes_pageno", "Invalid file descriptor");
		return ;
	}

	//????????????????????????
	//When the child node is a leaf node
	pmht_buf_ptr = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(pmht_buf_ptr, 0, MHT_BLOCK_SIZE);
	lmht_buf_ptr = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(lmht_buf_ptr, 0, MHT_BLOCK_SIZE);
	rmht_buf_ptr = (uchar*) malloc(MHT_BLOCK_SIZE);
	memset(rmht_buf_ptr, 0, MHT_BLOCK_SIZE);

	//????????????????????????????????????
	//Read the corresponding page number information to modify
	parent_offset = (*((int*)(mht_block_buf + MHT_BLOCK_OFFSET_POS))) *  MHT_BLOCK_SIZE + offset;
	//printf("offset : %d\t", offset);
	//printf("parent_offset : %d\t", parent_offset);
	fo_read_mht_block2(fd, pmht_buf_ptr, MHT_BLOCK_SIZE, parent_offset, SEEK_SET);

	lchild_offset = (*((int*)(pmht_buf_ptr + MHT_BLOCK_OFFSET_LCOS))) * MHT_BLOCK_SIZE + parent_offset;
	//printf("lchild_offset: %d\t", lchild_offset);
	fo_read_mht_block2(fd, lmht_buf_ptr, MHT_BLOCK_SIZE, lchild_offset, SEEK_SET);
	lchild_pgno = *((int*)(lmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO)) ;
	//printf("lchild_pgno : %d\t", lchild_pgno);

	rchild_offset = (*((int*)(pmht_buf_ptr + MHT_BLOCK_OFFSET_RCOS))) * MHT_BLOCK_SIZE + parent_offset;
	//printf("rchild_offset: %d\t", rchild_offset);
	fo_read_mht_block2(fd, rmht_buf_ptr, MHT_BLOCK_SIZE, rchild_offset , SEEK_SET);
	rchild_pgno = *((int*)(rmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO));
	//printf("rchild_pgno : %d\t", rchild_pgno);

	parent_pgno = lchild_pgno;
	//printf("parent_pgno : %d\t", parent_pgno);
	rmsl_pgno = rchild_pgno;
	//printf("rmsl_pgno : %d\n", rmsl_pgno);

	memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO, &parent_pgno, sizeof(int));
	memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_LCPN, &lchild_pgno, sizeof(int));
	memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_RCPN, &rchild_pgno, sizeof(int));
	memcpy(lmht_buf_ptr + MHT_BLOCK_OFFSET_PPN, &parent_pgno, sizeof(int));
	memcpy(rmht_buf_ptr + MHT_BLOCK_OFFSET_PPN, &parent_pgno, sizeof(int));

	fo_update_mht_block2(fd, pmht_buf_ptr, MHT_BLOCK_SIZE, parent_offset , SEEK_SET);
	fo_update_mht_block2(fd, lmht_buf_ptr, MHT_BLOCK_SIZE, lchild_offset, SEEK_SET);
	fo_update_mht_block2(fd, rmht_buf_ptr, MHT_BLOCK_SIZE, rchild_offset, SEEK_SET);
	//??????????????????????????????????????????
	//Update the page number information of its parent node along the path
	while((pmht_buf_ptr +MHT_BLOCK_OFFSET_RSVD)[0] != 0x01 && (temp_offset= *((int*)(pmht_buf_ptr +MHT_BLOCK_OFFSET_POS))) !=0)
	{
		memset(pmht_buf_ptr, 0, MHT_BLOCK_SIZE);
		memset(lmht_buf_ptr, 0, MHT_BLOCK_SIZE);
		memset(rmht_buf_ptr, 0, MHT_BLOCK_SIZE);

		parent_offset = temp_offset * MHT_BLOCK_SIZE + parent_offset;
		fo_read_mht_block2(fd, pmht_buf_ptr, MHT_BLOCK_SIZE, parent_offset, SEEK_SET);

		lchild_offset = (*((int*)(pmht_buf_ptr + MHT_BLOCK_OFFSET_LCOS))) * MHT_BLOCK_SIZE + parent_offset;
		fo_read_mht_block2(fd, lmht_buf_ptr, MHT_BLOCK_SIZE, lchild_offset, SEEK_SET);
		lchild_pgno = *((int*)(lmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO));
		//????????????RMSL?????????????????????????????????
		//Find the corresponding RMSL that is to find the page number of its rightmost subtree
		int block_offset = lchild_offset ;
		int temp_roffset = UNASSIGNED_OFFSET;
		uchar *temp_block_buf  = NULL;
		temp_block_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
		memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
		memcpy(temp_block_buf, lmht_buf_ptr, MHT_BLOCK_SIZE);
		while((temp_roffset = *((int*)(temp_block_buf + MHT_BLOCK_OFFSET_RCOS))) !=0)
		{
			//???????????????????????????
			//Read the right child node information
			block_offset = block_offset + temp_roffset * MHT_BLOCK_SIZE;
			//printf("block_offset:%d\n", block_offset);
			memset(temp_block_buf, 0, MHT_BLOCK_SIZE);
			fo_read_mht_block2(fd, temp_block_buf, MHT_BLOCK_SIZE, block_offset, SEEK_SET);
		}
		rmsl_pgno = *((int*)(temp_block_buf + MHT_BLOCK_OFFSET_PAGENO));
		free(temp_block_buf );

		rchild_offset = (*((int*)(pmht_buf_ptr + MHT_BLOCK_OFFSET_RCOS))) * MHT_BLOCK_SIZE + parent_offset;
		fo_read_mht_block2(fd, rmht_buf_ptr, MHT_BLOCK_SIZE, rchild_offset , SEEK_SET);
		rchild_pgno = *((int*)(rmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO));
		//printf("rchild_pgno : %d\t", rchild_pgno);

		parent_pgno = rmsl_pgno;
		//printf("parent_pgno : %d\t", parent_pgno);
		//printf("rmsl_pgno : %d\n", rmsl_pgno);

		memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_PAGENO, &parent_pgno, sizeof(int));
		memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_LCPN, &lchild_pgno, sizeof(int));
		memcpy(pmht_buf_ptr + MHT_BLOCK_OFFSET_RCPN, &rchild_pgno, sizeof(int));
		memcpy(lmht_buf_ptr + MHT_BLOCK_OFFSET_PPN, &parent_pgno, sizeof(int));
		memcpy(rmht_buf_ptr + MHT_BLOCK_OFFSET_PPN, &parent_pgno, sizeof(int));

		fo_update_mht_block2(fd, pmht_buf_ptr, MHT_BLOCK_SIZE, parent_offset , SEEK_SET);
		fo_update_mht_block2(fd, lmht_buf_ptr, MHT_BLOCK_SIZE, lchild_offset, SEEK_SET);
		fo_update_mht_block2(fd, rmht_buf_ptr, MHT_BLOCK_SIZE, rchild_offset, SEEK_SET);
	}

	free(pmht_buf_ptr);
	free(lmht_buf_ptr);
	free(rmht_buf_ptr);

	return;
}

int extentTheMHT(int fd)
{
    //????????????????????????
    //Fill the offset of the node
    int supplementaryNode_offset = 0;
    uchar *mhtblk_buffer = NULL;
    //???????????????
    //File header information
    PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
    //?????????????????????
    //Store root node information
    uchar *rootnode_buf = NULL;
    PQNode qnode_ptr = NULL;

    //????????????MHT??????????????????????????????
    //Find out if there are nodes that can be filled
    //1??????MHT????????????????????????????????????offset
    //1 Obtain the offset of the filling node through the MHT file header information
    if(fd < 3)
    {
        debug_print("extentTheMHT", "Invalid file descriptor");
        return -1;
    }

    //Reset the file hanlder
    lseek(fd, 0, SEEK_SET);

    if(!(mhtfilehdr_ptr = readMHTFileHeader(fd)))
    {
        debug_print("extentTheMHT", "Failed to read MHT file header");
        return -1;
    }
    supplementaryNode_offset = mhtfilehdr_ptr->m_firstSupplementaryLeafOffset;
    printf("FROM extentTheMHT: supplementaryNode_offset :%d\n",supplementaryNode_offset );

    g_mhtFileRootNodeOffset = mhtfilehdr_ptr->m_rootNodeOffset;
    if(supplementaryNode_offset == 0)
    {
        PQNode popped_qnode_ptr = NULL;
        //??????????????????????????????
        //Construct supplementary node to write file
        //a.???????????????????????????
        //a. Read the current root node information
        rootnode_buf = (uchar*) malloc(MHT_BLOCK_SIZE);
        memset(rootnode_buf, 0, MHT_BLOCK_SIZE);
        printf("rootNodeOffset: %d\n", mhtfilehdr_ptr->m_rootNodeOffset);
        fo_read_mht_block2(fd, rootnode_buf, MHT_BLOCK_SIZE, mhtfilehdr_ptr->m_rootNodeOffset, SEEK_SET);

        //b.??????????????????????????????????????????
        //b. Put the root node into the team and perform binary tree completion
        initQueue(&g_pQHeader, &g_pQ);
        qnode_ptr = mht_buffer_to_qnode(rootnode_buf, mhtfilehdr_ptr->m_rootNodeOffset, fd);
        if(!qnode_ptr)
        {
            return -1;
        }
        enqueue(&g_pQHeader, &g_pQ, qnode_ptr);
        //??????????????????????????????????????????????????????
        //Position the cursor to the position of the original root node for writing
        fo_locate_mht_pos(fd, mhtfilehdr_ptr->m_rootNodeOffset, SEEK_SET);
        deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ, fd);
        if(g_pQHeader->m_length > 1)
        {
            deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ, fd);
        }

        //????????????????????????????????????
        //Write the newly generated root node into the file
        popped_qnode_ptr = dequeue(&g_pQHeader, &g_pQ);
        check_pointer(popped_qnode_ptr, "popped_qnode_ptr");
        //??????????????????????????????
        //Create the root node by the queue node
        mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
        memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
        qnode_to_mht_buffer(popped_qnode_ptr, &mhtblk_buffer, MHT_BLOCK_SIZE);
        //?????????????????????????????????????????????????????????
        //Set the reserved area of the root node, which can be distinguished from other nodes
        (mhtblk_buffer + MHT_BLOCK_OFFSET_RSVD)[0] = 0x01;
        g_mhtFileRootNodeOffset = fo_locate_mht_pos(fd, 0, SEEK_CUR);
        //printf("g_mhtFileRootNodeOffset:%d\n",g_mhtFileRootNodeOffset);
        fo_update_mht_block(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
        free(mhtblk_buffer);
        mhtblk_buffer = NULL;
#ifdef PRINT_INFO_ENABLED
        print_qnode_info(popped_qnode_ptr);
#endif
        deleteQNode(&popped_qnode_ptr);

        //???????????????????????????
        //Update the supplementary node offset
        //supplementaryNode_offset = g_mhtFirstSplymtLeafOffset;
    }
    //???????????????????????????
    //Update the supplementary node offset
    supplementaryNode_offset = find_the_first_leaf_splymt_block_by_offset(fd, mhtfilehdr_ptr->m_rootNodeOffset);

    //??????MHT???????????????
    //update the MHT file header
    supplementaryNode_offset = find_the_first_leaf_splymt_block_by_offset(fd, supplementaryNode_offset);
    //printf("update  supplementaryNode_offset: %d\n", supplementaryNode_offset);
    uchar *mhthdr_buffer = NULL;
    mhthdr_buffer = (uchar*) malloc(MHT_HEADER_LEN);
    mhtfilehdr_ptr->m_rootNodeOffset = g_mhtFileRootNodeOffset;
    //printf("mhtfilehdr_ptr->m_rootNodeOffset: %d\n", mhtfilehdr_ptr->m_rootNodeOffset);
    mhtfilehdr_ptr->m_firstSupplementaryLeafOffset = supplementaryNode_offset == -1? 0 : supplementaryNode_offset;
    serialize_mht_file_header(mhtfilehdr_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
    fo_update_mht_file_header(fd, mhthdr_buffer, MHT_HEADER_LEN);

    free(mhthdr_buffer);
    free(rootnode_buf);
    free(mhtblk_buffer);
    freeMHTFileHeader(&mhtfilehdr_ptr);
    return supplementaryNode_offset;
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

	/* Moving file pointer to the position whence + rel_distance * MHT_BLOCK_SIZE */
	fo_locate_mht_pos(fd, rel_distance * MHT_BLOCK_SIZE, whence);
	bytes_read = read(fd, buffer, buffer_len);

	return bytes_read;
}

ssize_t fo_read_mht_block2(int fd, 
                            uchar *buffer, 
                            uint32 buffer_len, 
                            int offset,         // number of bytes from whence
                            int whence){
	ssize_t bytes_read = -1;

	if(fd < 0){
		debug_print("fo_read_mht_block2", "Invalid fd");
		return bytes_read;
	}

	if(!buffer || buffer_len != MHT_BLOCK_SIZE){
		check_pointer(buffer, "buffer");
		debug_print("fo_read_mht_block2", "Error parameters");
		return bytes_read;
	}

	/* Moving file pointer to the position: whence + offset */
	fo_locate_mht_pos(fd, offset, whence);
	bytes_read = read(fd, buffer, buffer_len);

	return bytes_read;
}

ssize_t fo_read_mht_file(int fd, 
                            uchar *buffer, 
                            uint32 buffer_len, 
                            int offset,         // number of bytes from whence
                            int whence){
	ssize_t bytes_read = -1;

	if(fd < 0){
		debug_print("fo_read_mht_file", "Invalid fd");
		return bytes_read;
	}

	if(!buffer || buffer_len <= 0){
		check_pointer(buffer, "buffer");
		debug_print("fo_read_mht_file", "Error parameters");
		return bytes_read;
	}

	/* Moving file pointer to the position: whence + offset */
	fo_locate_mht_pos(fd, offset, whence);
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

ssize_t fo_update_mht_block2(int fd, 
                             uchar *buffer, 
                             uint32 buffer_len, 
                             int offset,         
                             int whence){
	ssize_t bytes_write = -1;

	if(fd < 0){
		debug_print("fo_update_mht_block", "Invalid fd");
		return bytes_write;
	}

	if(!buffer || buffer_len != MHT_BLOCK_SIZE){
		check_pointer(buffer, "buffer");
		debug_print("fo_update_mht_block2", "Error parameters");
		return bytes_write;
	}

	fo_locate_mht_pos(fd, offset, whence);
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

off_t fo_search_mht_block_by_block_info(int fd,
                            PMHT_BLOCK mhtblk_ptr){
	return 0;
}

off_t fo_search_mht_block_by_qnode_info(int fd,
                            PQNode qnode_ptr){
	// we suppose that the file pointer is at the end of the file,
	// and the searching will proceed backwards (to the file header). 
	const char* THIS_FUNC_NAME = "fo_search_mht_block_by_qnode_info";
	uchar *mht_block_buf = NULL;
	uint32 mht_block_buf_len = MHT_BLOCK_SIZE;
	off_t fp_pos = 0;

	if(!qnode_ptr){
		check_pointer_ex(qnode_ptr, "qnode_ptr", THIS_FUNC_NAME, "null qnode_ptr");
		return 0;
	}

	if(fd < 0) {
		debug_print(THIS_FUNC_NAME, "invalid file handler fd");
		return 0;
	}

	mht_block_buf = (uchar*) malloc (MHT_BLOCK_SIZE);
	memset(mht_block_buf, 0, MHT_BLOCK_SIZE);

	while((fp_pos = fo_locate_mht_pos(fd, -MHT_BLOCK_SIZE, SEEK_CUR)) >= MHT_HEADER_LEN){
		//printf("FP_POS: %d\n", fp_pos);
		fo_read_mht_block2(fd, mht_block_buf, mht_block_buf_len, 0, SEEK_CUR);
		//print_hash_value(qnode_ptr->m_MHTNode_ptr->m_hash);
		//println();println();
		//print_hash_value((char*)(mht_block_buf+MHT_BLOCK_OFFSET_HASH));
		//println();println();
		if(compare_two_hashes(qnode_ptr->m_MHTNode_ptr->m_hash, (char*)(mht_block_buf+MHT_BLOCK_OFFSET_HASH))){
			// block found
			// NOTE: now the file pointer is at the end of the block
			mht_block_buf ? free(mht_block_buf) : nop();
#ifdef PRINT_INFO_ENABLED
			printf("BLOCK FOUND! FP_POS: %ld\n", fp_pos);
#endif
			return fp_pos;
		}
		fo_locate_mht_pos(fd, -MHT_BLOCK_SIZE, SEEK_CUR);
	}

	// not found
	mht_block_buf ? free(mht_block_buf) : nop();
	return 0;
}

void fo_print_mht_block(int fd, int whence){
	uchar *buffer_ptr = NULL;
	uint32 buffer_len = 0;
	uint32 bytes_read = 0;

	if(fd < 0){
		debug_print("fo_print_mht_block", "Invalid fd");
		return;
	}

	buffer_ptr = (uchar*) malloc (MHT_BLOCK_SIZE);
	memset(buffer_ptr, 0, MHT_BLOCK_SIZE);
	buffer_len = MHT_BLOCK_SIZE;

	bytes_read = fo_read_mht_block2(fd, buffer_ptr, buffer_len, 0, whence);
	print_mht_block(buffer_ptr, buffer_len);

	free(buffer_ptr);

	return;
}	

int fo_close_mhtfile(int fd){
	return close(fd);
}

int fo_copy_file(char* srcPath,char *destPath)
{
	int fd_src, fd_dest, ret;
	fd_src = open(srcPath, O_RDONLY);
		
	if(fd_src < 3)
	{
		perror("srcPath");
		return -1;
	}
	fd_dest = open(destPath, O_WRONLY|O_CREAT, 0755);
	if(fd_dest < 3)
	{
		close(fd_src);
		perror("destPath");
		return -1;
	}
	char buf[1024] = "";
	do
	{
    	memset(buf,0,sizeof(buf));
		ret = read(fd_src, buf, sizeof(buf));
		if(ret>0)
			write(fd_dest, buf, ret);
	}while(ret >0);
	close(fd_src);
	close(fd_dest);
	return 0;
}

void fo_printMHTFile(int fd)
{
	uchar *mhtblk_buffer = NULL;
    PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
    PMHT_BLOCK tmpblk_ptr = NULL;

	if(fd < 0){
		debug_print("fo_printMHTFile", "Invalid fd");
		return;
	}

    tmpblk_ptr = makeMHTBlock();
    if(!(mhtfilehdr_ptr = readMHTFileHeader(fd)))
    {
        debug_print("insertpageinmht", "Failed to read MHT file header");
        return;
    }

    printf("FSLLOS: %d,   RNO:%d\n",mhtfilehdr_ptr->m_firstSupplementaryLeafOffset, mhtfilehdr_ptr->m_rootNodeOffset);
    mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
    while(fo_locate_mht_pos(fd, 0, SEEK_CUR) != mhtfilehdr_ptr->m_rootNodeOffset)
    {
        memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
        fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
        unserialize_mht_block(mhtblk_buffer,  MHT_BLOCK_SIZE, &tmpblk_ptr);
        printf("pgno:%d\t level: %d\t ppgno:%d\t  lpgno:%d\t rpgno:%d\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_nodeLevel, tmpblk_ptr->m_parentPageNo, tmpblk_ptr->m_lChildPageNo, tmpblk_ptr->m_rChildPageNo);
		//printf("pgno:%d\t ISN: %hhu\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_isSupplementaryNode);
    }

    // output root block
    memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
    fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
    unserialize_mht_block(mhtblk_buffer,  MHT_BLOCK_SIZE, &tmpblk_ptr);
    printf("pgno:%d\t level: %d\t ppgno:%d\t  lpgno:%d\t rpgno:%d\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_nodeLevel, tmpblk_ptr->m_parentPageNo, tmpblk_ptr->m_lChildPageNo, tmpblk_ptr->m_rChildPageNo);
}


/*---------- End of File Operation Functions  ------------*/

/*--------------- Misc. Functions ----------------*/


/*--------------- End of Misc. Functions ----------------*/