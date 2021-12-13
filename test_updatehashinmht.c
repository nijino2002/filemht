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
	uchar hash_string[HASH_STR_LEN] = {0};
	uchar *new_hash = NULL;
	PMHT_BLOCK mhtblk_ptr = NULL;
	SHA256_CTX ctx;
	int block_offset = -1;
	int fd = 0;

	if((fd = initOpenMHTFileWR(MHT_DEFAULT_FILE_NAME)) < 2){
		printf("Failed to open file %s\n", MHT_DEFAULT_FILE_NAME);
		exit(0);
	}

	printf("Input page number to be update: ");
	scanf("%d", &pageNo);
	mhtblk_ptr = searchPageByNo(pageNo);
	if(mhtblk_ptr){
		convert_hash_to_string(mhtblk_ptr->m_hash, hash_string, HASH_STR_LEN);
		printf("Found page: %d, and the hash value is: %s\n", mhtblk_ptr->m_pageNo, hash_string);
		freeMHTBlock(&mhtblk_ptr);
	}
	else{
		printf("No page found.\n");
		fo_close_mhtfile(g_mhtFileFdRd);
		exit(0);
	}

	// compute a new hash value
	new_hash = (uchar*) malloc(HASH_LEN);
	memset(new_hash, 0, HASH_LEN);
	sha256_init(&ctx);
	sha256_update(&ctx, "MYHASH1", strlen("MYHASH1"));
	sha256_final(&ctx, new_hash);
	block_offset = updateMHTBlockHashByPageNo(pageNo, new_hash, HASH_LEN, fd);
	if(block_offset <= 0) {
		printf("Update failed.\n");
		fo_close_mhtfile(g_mhtFileFdRd);
		exit(0);
	}
	printf("the return value:%d\n", block_offset);
	//memset(new_hash, 0, HASH_LEN);
	// read the new hash value from MHT file
	//fo_read_mht_file(g_mhtFileFdRd, new_hash, HASH_LEN, block_offset + MHT_BLOCK_OFFSET_HASH, SEEK_SET);
	//memset(hash_string, 0, HASH_STR_LEN);
	//convert_hash_to_string(new_hash, hash_string, HASH_STR_LEN);
	//printf("The new hash value: %s\n", hash_string);

	free(new_hash);
	fo_close_mhtfile(g_mhtFileFdRd);
	
	return 0;
}
