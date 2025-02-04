#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "sha256.h"
#include "ds.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"
#include "mhtfile_ex.h"

int main(int argc, char const *argv[])
{
	int ret_val = RETCODE_OK;
	const int out_hash_string_len = SHA256_BLOCK_SIZE * 2 + 1;
	BYTE *out_hash_string = NULL;
	BYTE *out_hash = NULL;
	char mhtfile_prefix[20] = {0};

	if(argc < 2){
		printf("Usage: %s [in_ds_filename]\n", argv[0]);
		return 0;
	}

	out_hash_string = (BYTE*) malloc (out_hash_string_len);
	out_hash = (BYTE*) malloc (SHA256_BLOCK_SIZE);
	memset(out_hash_string, 0, out_hash_string_len);
	memset(out_hash, 0, SHA256_BLOCK_SIZE);
	sha256_file(argv[1], out_hash);
	convert_hash_to_string(out_hash, out_hash_string, out_hash_string_len);
	printf("File hash is %s.\n", out_hash_string);
	str_substring(out_hash_string, mhtfile_prefix, 0, 16);
	printf("MHT file prefix: %s.\n", mhtfile_prefix);

	ret_val = buildMHTFileFvByFixedLeaves(argv[1], mhtfile_prefix, FALSE, 16);
	if(ret_val != 0) {
		printf("Some errors occurred. ERROR CODE: %d.\n", ret_val);
	}
	else {
		printf("Executing buildMHTFileFvByFixedLeaves() OK.\n");
	}

	free(out_hash_string);
	free(out_hash);

	return 0;
}