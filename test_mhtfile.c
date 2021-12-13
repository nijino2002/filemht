#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"

extern PQNode g_pQHeader;
extern PQNode g_pQ;

/*
Testing functions of queue
 */
void test_build_mhtfile();

void test_build_mhtfile(){
	PQNode popped_qnode_ptr = NULL;
	PMHT_FILE_HEADER mht_file_header_ptr = NULL;
	uchar *mhtblk_buffer = NULL;
	uchar *mhthdr_buffer = NULL;

	//创建mht文件
	if((g_mhtFileFD = fo_create_mhtfile(MHT_DEFAULT_FILE_NAME)) < 0) {
		debug_print("buildMHTFile", "Creating MHT file failed!");
		return;
	}

	//保留文件头的128bytes
	if(fo_locate_mht_pos(g_mhtFileFD, MHT_HEADER_LEN, SEEK_CUR) < 0) {
		debug_print("buildMHTFile", "Reserving space for root node failed!");
		return;
	}

	//初始化文件头信息
	mht_file_header_ptr = makeMHTFileHeader();
	
	//处理数据信息，将其入队同时进行补充使其构成完全二叉树
	process_all_pages(&g_pQHeader, &g_pQ);
	//printf("\n\n\n g_pQHeader->next->m_level:%d g_pQ->m_level: %d", g_pQHeader->next->m_level, g_pQ->m_level);
	//printf("\n g_pQHeader->m_length:%d\n\n\n", g_pQHeader->m_level);
	
	//如何初始化构造为2的n次，则不需要进行填充，此时队列剩余的就是root节点
	if(g_pQHeader->m_level > 1)
	{
		deal_with_remaining_nodes_in_queue(&g_pQHeader, &g_pQ, g_mhtFileFD);
	}
	
	//printf("\n\n\n g_pQHeader->next->m_level:%d g_pQ->m_level:%d", g_pQHeader->next->m_level, g_pQ->m_level);
	//printf("\n g_pQHeader->m_length:%d\n\n\n", g_pQHeader->m_level);


	//将root节点写入文件
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

		print_qnode_info(popped_qnode_ptr);
		// free node
		deleteQNode(&popped_qnode_ptr);
	} //while

	//初始构造后，更新文件头信息
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

int main(int argc, char const *argv[])
{
	char hash1[40] = {0};
	char hash2[40] = {0};
	char combinedHash[40] = {0};
	char hash_string[65] = {0};

	test_build_mhtfile();

	return 0;
}
