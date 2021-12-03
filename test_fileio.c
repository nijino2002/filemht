#include "mhtfile.h"

int main(int argc, char const *argv[])
{
	int fd = -1;
	int i = 1;
	PMHT_FILE_HEADER mht_fh_ptr = NULL;
	PMHT_BLOCK mht_blk_ptr = NULL;
	uchar *mht_fh_buffer = NULL;
	uchar *mht_blk_buffer = NULL;

	fd = fo_create_mhtfile("./testdbfile.db");
	mht_fh_ptr = makeMHTFileHeader();
	mht_blk_ptr = makeMHTBlock();
	mht_blk_ptr->m_pageNo = UNASSIGNED_PAGENO;

	mht_fh_buffer = (uchar*) malloc(sizeof(uchar) * MHT_HEADER_LEN);
	mht_blk_buffer = (uchar*) malloc(sizeof(uchar) * MHT_BLOCK_SIZE);
	memset(mht_fh_buffer, 0, MHT_HEADER_LEN);

	serialize_mht_file_header(mht_fh_ptr, &mht_fh_buffer, MHT_HEADER_LEN);
	fo_update_mht_file_header(fd, mht_fh_buffer, MHT_HEADER_LEN);

	for(i = 0; i < 100; i++){
		mht_blk_ptr->m_pageNo = i + 1;
		memset(mht_blk_buffer, 0, MHT_BLOCK_SIZE);
		serialize_mht_block(mht_blk_ptr, &mht_blk_buffer, MHT_BLOCK_SIZE);
		fo_update_mht_block(fd, mht_blk_buffer, MHT_BLOCK_SIZE, 0, SEEK_CUR);
	}

	fo_close_mhtfile(fd);

	freeMHTFileHeader(&mht_fh_ptr);
	freeMHTBlock(&mht_blk_ptr);
	free(mht_fh_buffer);
	free(mht_blk_buffer);

	return 0;
}