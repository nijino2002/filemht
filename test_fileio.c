#include "mhtfile.h"

int main(int argc, char const *argv[])
{
	int fd = -1;
	int i = 1;
	PMHT_HEADER_BLOCK mhthdrblk_ptr = NULL;
	PMHT_CHILD_NODE_BLOCK mhtcldnodeblk_ptr = NULL;
	uchar *mhthdr_buffer = NULL;
	uchar *mhtcldnd_buffer = NULL;

	fd = fo_create_mhtfile("./testdbfile.db");
	mhthdrblk_ptr = makeMHTHeaderBlock();
	mhtcldnodeblk_ptr = makeMHTChildNodeBlock();
	mhthdrblk_ptr->m_nodeBlock.m_pageNo = 0;

	mhthdr_buffer = (uchar*) malloc(sizeof(uchar) * MHT_HEADER_LEN);
	mhtcldnd_buffer = (uchar*) malloc(sizeof(uchar) * MHT_CNB_LEN);
	memset(mhthdr_buffer, 0, MHT_HEADER_LEN);

	serialize_mhthdr_block(mhthdrblk_ptr, &mhthdr_buffer, MHT_HEADER_LEN);
	fo_update_mht_header_block(fd, mhthdr_buffer, MHT_HEADER_LEN);

	for(i = 0; i < 100; i++){
		mhtcldnodeblk_ptr->m_nodeBlock.m_pageNo = i + 1;
		memset(mhtcldnd_buffer, 0, MHT_CNB_LEN);
		serialize_cldnode_block(mhtcldnodeblk_ptr, &mhtcldnd_buffer, MHT_CNB_LEN);
		fo_update_mht_child_node_block(fd, mhtcldnd_buffer, MHT_CNB_LEN, 0, SEEK_CUR);
	}

	fo_close_mhtfile(fd);

	freeMHTHeaderBlock(&mhthdrblk_ptr);
	freeMHTChildNodeBlock(&mhtcldnodeblk_ptr);
	free(mhthdr_buffer);
	free(mhtcldnd_buffer);

	return 0;
}