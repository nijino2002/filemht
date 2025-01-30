#include "ds.h"

int main(int argc, char const *argv[])
{
	if(ds_create_dataset_random("./myds.ds", 16, 10) == RETCODE_OK)
		printf("Successfully created dataset file %s.\n", "myds.ds");
	else
		printf("Failed to create dataset file.\n");
	return 0;
}