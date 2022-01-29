#include "sha256.h"
#include "mhtdefs.h"

PMHTNode makeMHTNode(int pageno, const char d[]){
	PMHTNode node_ptr = NULL;
	if(d == NULL)
		return NULL;
	node_ptr = (PMHTNode) malloc(sizeof(MHTNode));
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_pageNo = pageno;
	memcpy(node_ptr->m_hash, d, HASH_LEN);	// HASH_LEN == SHA256_BLOCK_SIZE == 32

	node_ptr->m_lchildPageNo = node_ptr->m_rchildPageNo = node_ptr->m_parentPageNo = UNASSIGNED_PAGENO;
	node_ptr->m_lchildOffset = node_ptr->m_rchildOffset = node_ptr->m_parentOffset = UNASSIGNED_OFFSET;
	//node_ptr->m_lchildOffset = node_ptr->m_lchildPageNo = UNASSIGNED_PAGENO;
	//node_ptr->m_rchildOffset = node_ptr->m_rchildPageNo = UNASSIGNED_PAGENO;
	//node_ptr->m_parentOffset = node_ptr->m_parentPageNo = UNASSIGNED_PAGENO;

	return node_ptr;
}

PMHTNode makeZeroMHTNode(int pageno){
	return makeMHTNode(pageno, g_zeroHash);
}

void deleteMHTNode(PMHTNode *node_ptr){
	if(*node_ptr){
		free(*node_ptr);
		*node_ptr = NULL;
	}

	return;
}

void generateHashByPageNo_SHA256(int page_no, char *buf, uint32 buf_len){
	char tmp_buf[32]={0};

	if(page_no < 0){
		printf("Page number must larger than 0.\n");
		return;
	}

	if(!buf || buf_len < SHA256_BLOCK_SIZE){
		printf("buf cannot be NULL and buf_len must larger than 32 bytes.\n");
		return;
	}

	sprintf(tmp_buf, "%d", page_no);
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, tmp_buf, strlen(tmp_buf));
	sha256_final(&ctx, buf);

	return;
}

void generateHashByBuffer_SHA256(char *in_buf, uint32 in_buf_len, char *buf, uint32 buf_len){
	if(!in_buf || !buf || buf_len < SHA256_BLOCK_SIZE){
		printf("in_buf and buf cannot be NULL and buf_len must larger than 32 bytes.\n");
		return;
	}

	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, in_buf, in_buf_len);
	sha256_final(&ctx, buf);

	return;
}

void generateCombinedHash_SHA256(char *hash1, char *hash2, char *buf, uint32 buf_len){
	char tmp_buf[SHA256_BLOCK_SIZE * 2 + 1] = {0};

	if(!hash1 || !hash2){
		printf("Parameters \"hash1\" and \"hash2\" cannot be NULL.\n");
		return;
	}

	if(!buf || buf_len < SHA256_BLOCK_SIZE){
		printf("buf cannot be NULL and buf_len must larger than 32 bytes.\n");
		return;
	}

	memcpy(tmp_buf, hash1, SHA256_BLOCK_SIZE);
	memcpy(tmp_buf + SHA256_BLOCK_SIZE, hash2, SHA256_BLOCK_SIZE);
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, tmp_buf, strlen(tmp_buf));
	sha256_final(&ctx, buf);

	return;
}
