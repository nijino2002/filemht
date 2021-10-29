#include "mhtdefs.h"

PMHTNode makeMHTNode(uint32 pageno, const char d[]){
	PMHTNode node_ptr = NULL;
	if(pageno <= 0 || d == NULL)
		return NULL;
	node_ptr = (PMHTNode) malloc(sizeof(MHTNode));
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_pageNo = pageno;
	memcpy(node_ptr->m_hash, d, HASH_LEN);
	node_ptr->m_lchildOffset = node_ptr->m_lchildPageNo = 0;
	node_ptr->m_rchildOffset = node_ptr->m_rchildPageNo = 0;
	node_ptr->m_parentOffset = node_ptr->m_parentPageNo = 0;

	return node_ptr;
}

void deleteMHTNode(PMHTNode *node_ptr){
	if(*node_ptr){
		free(*node_ptr);
		*node_ptr = NULL;
	}

	return;
}