#include "defs.h"
#include "mhtdefs.h"
#include "sha256.h"
#include "dbqueue.h"
#include "qnode_pool.h"  // 引入对象池接口

PQNode makeQHeader() {
	PQNode node_ptr = pool_alloc_qnode();
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_length = 0;
	node_ptr->m_is_written = FALSE;
	node_ptr->m_MHTNode_ptr = NULL;
	node_ptr->m_is_supplementary_node = (uchar) FALSE;
	node_ptr->m_is_zero_node = (uchar) FALSE;
	node_ptr->m_RMSTL_page_no = UNASSIGNED_PAGENO;
	node_ptr->prev = NULL;
	node_ptr->next = NULL;
	return node_ptr;
}

PQNode makeQNode(PMHTNode pmhtnode, uint16 level){
	if(pmhtnode == NULL || level < 0)
		return NULL;
	PQNode node_ptr = pool_alloc_qnode();
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_level = level;
	node_ptr->m_is_written = FALSE;
	node_ptr->m_MHTNode_ptr = pmhtnode;
	node_ptr->m_is_supplementary_node = (uchar)(pmhtnode->m_pageNo >= UNASSIGNED_INDEX);
	node_ptr->m_is_zero_node = (uchar) FALSE;
	node_ptr->m_RMSTL_page_no = UNASSIGNED_PAGENO;
	node_ptr->prev = NULL;
	node_ptr->next = NULL;
	return node_ptr;
}

PQNode makeQNode2(PMHTNode pmhtnode, uint16 level, uchar ISN, uchar IZN, int RMSTLPN) {
	if(pmhtnode == NULL || level < 0)
		return NULL;
	PQNode node_ptr = pool_alloc_qnode();
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_level = level;
	node_ptr->m_is_written = FALSE;
	node_ptr->m_MHTNode_ptr = pmhtnode;
	node_ptr->m_is_supplementary_node = ISN;
	node_ptr->m_is_zero_node = IZN;
	node_ptr->m_RMSTL_page_no = RMSTLPN;
	node_ptr->prev = NULL;
	node_ptr->next = NULL;
	return node_ptr;
}

PQNode makeCombinedQNodeFromSingleNode(PQNode node_ptr) {
	return makeCombinedQNode(node_ptr, 
		makeQNode(makeMHTNode(SINGLENODECMB_PAGENO, ZERO_STR), node_ptr->m_level + 1));
}

PQNode makeCombinedQNode(PQNode node1_ptr, PQNode node2_ptr) {
	if(!node1_ptr || !node2_ptr){
		check_pointer(node1_ptr, "node1_ptr");
		check_pointer(node2_ptr, "node2_ptr");
		return NULL;
	}

	char tmp_buf[HASH_LEN] = {0};
	generateCombinedHash_SHA256(node1_ptr->m_MHTNode_ptr->m_hash,
								node2_ptr->m_MHTNode_ptr->m_hash,
								tmp_buf, HASH_LEN);
	PMHTNode new_mhtnode_ptr = makeMHTNode(UNASSIGNED_PAGENO, tmp_buf);
	check_pointer(new_mhtnode_ptr, "new_mhtnode_ptr");
	PQNode new_qnode_ptr = makeQNode(new_mhtnode_ptr, node1_ptr->m_level + 1);
	check_pointer(new_qnode_ptr, "new_qnode_ptr");
	return new_qnode_ptr;
}

void deleteQNode(PQNode *node_ptr){
	if(*node_ptr){
		if((*node_ptr)->m_MHTNode_ptr)
			deleteMHTNode(&((*node_ptr)->m_MHTNode_ptr));
		pool_free_qnode(*node_ptr);
		*node_ptr = NULL;
	}
}

PQNode lookBackward(PQNode pNode){
	return (pNode && pNode->prev) ? pNode->prev : NULL;
}

void initQueue(PQNode *pQHeader, PQNode *pQ){
	if(*pQHeader)
		pool_free_qnode(*pQHeader);
	*pQHeader = makeQHeader();
	*pQ = *pQHeader;
}

PQNode enqueue(PQNode *pQHeader, PQNode *pQ, PQNode pNode){
	if(!(*pQHeader) || !(*pQ) || !pNode)
		return NULL;
	(*pQ)->next = pNode;
	pNode->prev = *pQ;
	*pQ = pNode;
	pNode->next = NULL;
	(*pQHeader)->m_length++;
	return pNode;
}

PQNode dequeue(PQNode *pQHeader, PQNode *pQ){
	if(*pQ == *pQHeader){
		printf("Empty queue.\n");
		return NULL;
	}
	PQNode tmp_ptr = (*pQHeader)->next;
	(*pQHeader)->next = tmp_ptr->next;
	if(tmp_ptr->next)
		tmp_ptr->next->prev = *pQHeader;
	else
		*pQ = *pQHeader;
	if((*pQHeader)->m_length > 0) (*pQHeader)->m_length--;
	return tmp_ptr;
}

PQNode dequeue_sub(PQNode *pQHeader, PQNode *pQ){
	if(*pQ == *pQHeader){
		printf("Empty queue.\n");
		return NULL;
	}
	PQNode first = (*pQHeader)->next;
	PQNode tmp_ptr = first->next;
	if(!tmp_ptr){
		check_pointer_ex(tmp_ptr, "tmp_ptr", "dequeue_sub", "null second element pointer");
		return NULL;
	}
	first->next = tmp_ptr->next;
	if(tmp_ptr->next)
		tmp_ptr->next->prev = first;
	else
		*pQ = first;
	if((*pQHeader)->m_length > 0) (*pQHeader)->m_length--;
	return tmp_ptr;
}

PQNode dequeue_sppos(PQNode *pQHeader, PQNode *pQ, PQNode pos) {
	if(*pQ == *pQHeader || pos == *pQHeader){
		printf("Invalid dequeue position.\n");
		return NULL;
	}
	if(pos->next){
		pos->prev->next = pos->next;
		pos->next->prev = pos->prev;
	}
	else{
		return dequeue(pQHeader, pQ);
	}
	if((*pQHeader)->m_length > 0) (*pQHeader)->m_length--;
	return pos;
}

PQNode peekQueue(PQNode pQHeader){
	return (pQHeader && pQHeader->next) ? pQHeader->next : NULL;
}

void freeQueue(PQNode *pQHeader, PQNode *pQ) {
	if(!(*pQHeader)) return;
	PQNode tmp_ptr = (*pQHeader)->next;
	while(tmp_ptr = dequeue(pQHeader, pQ)){
		if(tmp_ptr->m_MHTNode_ptr)
			free(tmp_ptr->m_MHTNode_ptr);
		pool_free_qnode(tmp_ptr);
	}
	pool_free_qnode(*pQHeader);
	*pQHeader = NULL;
	*pQ = NULL;
}

void freeQueue2(PQNode *pQHeader){
	if(!(*pQHeader)) return;
	PQNode tmp_ptr = (*pQHeader)->next;
	while(tmp_ptr){
		(*pQHeader)->next = tmp_ptr->next;
		if(tmp_ptr->next)
			tmp_ptr->next->prev = *pQHeader;
		if(tmp_ptr->m_MHTNode_ptr)
			free(tmp_ptr->m_MHTNode_ptr);
		pool_free_qnode(tmp_ptr);
		tmp_ptr = (*pQHeader)->next;
	}
	pool_free_qnode(*pQHeader);
	*pQHeader = NULL;
}

void freeQueue3(PQNode *pQ) {
	if(!(*pQ)) return;
	PQNode tmp_ptr = *pQ;
	while(tmp_ptr && tmp_ptr->prev)
		tmp_ptr = tmp_ptr->prev;
	freeQueue2(&tmp_ptr);
}

void printQueue(PQNode pQHeader) {
	if(!pQHeader){
		check_pointer(pQHeader, "printQueue: pQHeader");
		return;
	}
	PQNode tmp_ptr = pQHeader->next;
	uint32 i = 1;
	while(tmp_ptr){
		printf("%d: PageNo-Level: %d-%d\n", i++, tmp_ptr->m_MHTNode_ptr->m_pageNo, tmp_ptr->m_level);
		tmp_ptr = tmp_ptr->next;
	}
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
}

void print_qnode_info_ex(PQNode qnode_ptr, uint32 flags){
	if(!qnode_ptr){
		check_pointer_ex(qnode_ptr, "qnode_ptr", "printQNode", "null qnode_ptr");
		return;
	}
	printf("[");
	if(flags & PRINT_QNODE_FLAG_INDEX){
		printf("index: %d, ", qnode_ptr->m_MHTNode_ptr->m_pageNo);
	}
	if(flags & PRINT_QNODE_FLAG_HASH){
		print_hash_value(qnode_ptr->m_MHTNode_ptr->m_hash);
	}
	printf("]");
}
