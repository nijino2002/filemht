#include "ds.h"

#define MY_BLOCK_SIZE	16
#define MY_BLOCK_NUM	10000

int main(int argc, char const *argv[])
{
	DS_HEADER ds_hdr = {{0}, 0, 0};

	if(ds_create_dataset_random("./myds.ds", MY_BLOCK_SIZE, MY_BLOCK_NUM) == RETCODE_OK)
		printf("Successfully created dataset file %s.\n", "myds.ds");
	else
		printf("Failed to create dataset file.\n");

	if(!ds_verify_ds("./myds.ds", &ds_hdr)){
		printf("Failed to verify ds file.\n");
	}
	else {
		printf("Successfully verify ds file.\n");
	}

	return 0;
}
