#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"

extern PQNode g_pQHeader;
extern PQNode g_pQ;

int main(int argc, char const *argv[])
{
	int pageNo = UNASSIGNED_PAGENO;
	PMHT_BLOCK mhtblk_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;
	PQNode qnode_ptr = NULL;
	char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};

	if(argc < 2){
		printf("Usage: %s [MHT file name]\n", argv[0]);
		return -1;
	}

	//插入数据
	for(int i=1; i<35;  i+=14){
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(i, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		printf("insert %d page.\n", i);
		if(insertNewPageDisorder(i, tmp_hash_buffer, SHA256_BLOCK_SIZE, argv[1])<0)
		{
			printf("insertNewPageDisorder failed.\n");
		}
	}
	
	return 0;
}
