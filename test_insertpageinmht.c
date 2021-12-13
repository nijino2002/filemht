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

	int fd = 0;
	if( (fd = initOpenMHTFileWR(MHT_DEFAULT_FILE_NAME))  < 2){
		printf("Failed to open file %s\n", MHT_DEFAULT_FILE_NAME);
		exit(0);
	}
	//查看插入结果

	uchar *mhtblk_buffer = NULL;
    PMHT_FILE_HEADER mhtfilehdr_ptr = NULL;
	PMHT_BLOCK tmpblk_ptr = NULL;
	tmpblk_ptr = makeMHTBlock();
	if(!(mhtfilehdr_ptr = readMHTFileHeader()))
    {
		debug_print("insertpageinmht", "Failed to read MHT file header");
		return -1;
	}

	printf("FSLLOS: %d,   RNO:%d\n",mhtfilehdr_ptr->m_firstSupplementaryLeafOffset, mhtfilehdr_ptr->m_rootNodeOffset);
	//fo_locate_mht_pos(fd, MHT_HEADER_LEN, SEEK_SET);
	mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
	while(fo_locate_mht_pos(fd, 0, SEEK_CUR) != mhtfilehdr_ptr->m_rootNodeOffset)
	{
		memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
		fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
		unserialize_mht_block(mhtblk_buffer,  MHT_BLOCK_SIZE, &tmpblk_ptr);
		printf("pgno:%d\t level: %d\t ppgno:%d\t pos:%d\t lpgno:%d\t los: %d\t rpgno:%d\t ros: %d\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_nodeLevel, tmpblk_ptr->m_parentPageNo, tmpblk_ptr->m_parentOffset, tmpblk_ptr->m_lChildPageNo, tmpblk_ptr->m_lChildOffset, tmpblk_ptr->m_rChildPageNo, tmpblk_ptr->m_rChildOffset);
	}

	//插入数据
	for(int i=4; i<8;  i++){
		memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
		generateHashByPageNo_SHA256(i, tmp_hash_buffer, SHA256_BLOCK_SIZE);
		mhtnode_ptr = makeMHTNode(i, tmp_hash_buffer); 
		check_pointer((void*)mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF); 
		check_pointer((void*)qnode_ptr, "qnode_ptr");
		mhtblk_ptr = makeMHTBlock();
		convert_qnode_to_mht_block(qnode_ptr, &mhtblk_ptr);
		insertNewMHTBlock(mhtblk_ptr, fd);
		free(mhtnode_ptr);
		mhtnode_ptr = NULL;
		free(qnode_ptr);
		qnode_ptr = NULL;
		free(mhtblk_ptr);
		mhtblk_ptr = NULL;
	}

	if(!(mhtfilehdr_ptr = readMHTFileHeader()))
    {
		debug_print("insertpageinmht", "Failed to read MHT file header");
		return -1;
	}
	printf("FSLLOS: %d,   RNO:%d\n",mhtfilehdr_ptr->m_firstSupplementaryLeafOffset, mhtfilehdr_ptr->m_rootNodeOffset);
	fo_locate_mht_pos(fd, MHT_HEADER_LEN, SEEK_SET);
	mhtblk_buffer = (uchar*) malloc(MHT_BLOCK_SIZE);
	while(fo_locate_mht_pos(fd, 0, SEEK_CUR) != mhtfilehdr_ptr->m_rootNodeOffset)
	{
		memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
		memset(tmpblk_ptr, 0, sizeof(MHT_BLOCK));
		fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
		unserialize_mht_block(mhtblk_buffer,  MHT_BLOCK_SIZE, &tmpblk_ptr);
		printf("pgno:%d\t level: %d\t ppgno:%d\t pos:%d\t lpgno:%d\t los: %d\t rpgno:%d\t ros: %d\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_nodeLevel, tmpblk_ptr->m_parentPageNo, tmpblk_ptr->m_parentOffset, tmpblk_ptr->m_lChildPageNo, tmpblk_ptr->m_lChildOffset, tmpblk_ptr->m_rChildPageNo, tmpblk_ptr->m_rChildOffset);
	}

	printf("当前根节点信息：");
	memset(mhtblk_buffer, 0, MHT_BLOCK_SIZE);
	memset(tmpblk_ptr, 0, sizeof(MHT_BLOCK));
	fo_read_mht_block2(fd, mhtblk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	unserialize_mht_block(mhtblk_buffer,  MHT_BLOCK_SIZE, &tmpblk_ptr);
	printf("pgno:%d\t level: %d\t ppgno:%d\t pos:%d\t lpgno:%d\t los: %d\t rpgno:%d\t ros: %d\n",tmpblk_ptr->m_pageNo, tmpblk_ptr->m_nodeLevel, tmpblk_ptr->m_parentPageNo, tmpblk_ptr->m_parentOffset, tmpblk_ptr->m_lChildPageNo, tmpblk_ptr->m_lChildOffset, tmpblk_ptr->m_rChildPageNo, tmpblk_ptr->m_rChildOffset);
	
	fo_close_mhtfile(fd);	
	return 0;
}
