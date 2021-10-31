#include "sha256.h"
#include "mhtdefs.h"

PMHTNode makeMHTNode(uint32 pageno, const char d[]){
	PMHTNode node_ptr = NULL;
	if(pageno <= 0 || d == NULL)
		return NULL;
	node_ptr = (PMHTNode) malloc(sizeof(MHTNode));
	if(node_ptr == NULL)
		return NULL;
	node_ptr->m_pageNo = pageno;
	memcpy(node_ptr->m_hash, d, HASH_LEN);	// HASH_LEN == SHA256_BLOCK_SIZE == 32
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

void generateHashByPageNo_SHA256(uint32 page_no, char *buf, uint32 buf_len){
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
