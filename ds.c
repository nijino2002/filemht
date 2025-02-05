#include "ds.h"

const char DS_VERSION[DS_VERSION_LEN] = "ds_v1.0";

uint32 ds_create_dataset_random(char* filename, uint32 block_size, uint32 block_num){
	const char* THIS_FUNC_NAME = "ds_create_dataset_random";
	uint32 ret_val = RETCODE_OK;
	int fd = -1;
	int open_flags;
	mode_t file_perms;
	int i = 0;
	int index = 0;
	char* gen_str = NULL;
	char* buffer = NULL;
	int buffer_len = sizeof(int) + block_size;

	check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
	block_num <= 0 || block_size <= 0 ? debug_print(THIS_FUNC_NAME, "neither block_num nor block_size can be <= 0") : nop();

	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	fd = open(filename, open_flags, file_perms);
	if(fd < 0){
		debug_print(THIS_FUNC_NAME, "failed to open file");
		ret_val = RETCODE_FAILED_TO_OPEN_FILE;
		return ret_val;
	}

	buffer = (char*) malloc (buffer_len);
	if(!buffer){
		debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
		ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
		return ret_val;
	}

	// write ds version info.
	write(fd, DS_VERSION, DS_VERSION_LEN);

	// write ds block length (DL)
	write(fd, &buffer_len, sizeof(int));

	// write blocks
	for(i = 0; i < block_num; i++){
		index = i + 1;
		memset(buffer, 0, buffer_len);
		memcpy(buffer, &index, sizeof(uint32));
		gen_str = generate_random_string(block_size);
		memcpy(buffer + sizeof(uint32), gen_str, block_size);
		free(gen_str); gen_str = NULL;
		write(fd, buffer, buffer_len);
	}

	free(buffer); buffer = NULL;
	close(fd);

	return ret_val;
}

uint32 ds_create_dataset_by_array(char* filename, void* array_ptr, uint32 array_elem_size, uint32 array_length){
	const char* THIS_FUNC_NAME = "ds_create_dataset_by_array";
	uint32 ret_val = 0;
	int fd = -1;
	int open_flags;
	mode_t file_perms;
	int i = 0;
	int index = 0;
	char* buffer = NULL;
	int buffer_len = sizeof(int) + array_elem_size;

	check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
	check_pointer_ex(array_ptr, "array_ptr", THIS_FUNC_NAME, "null array pointer");
	array_elem_size <= 0 || array_length <= 0 ? debug_print(THIS_FUNC_NAME, "neither array_elem_size nor array_length can be <= 0") : nop();

	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	fd = open(filename, open_flags, file_perms);
	if(fd < 0){
		debug_print(THIS_FUNC_NAME, "failed to open file");
		ret_val = RETCODE_FAILED_TO_OPEN_FILE;
		return ret_val;
	}

	buffer = (char*) malloc (buffer_len);
	if(!buffer){
		debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
		ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
		return ret_val;
	}

	// write ds version info.
	write(fd, DS_VERSION, DS_VERSION_LEN);

	// write ds block length (DL)
	write(fd, &buffer_len, sizeof(int));

	for (i = 0; i < array_length; i++){
		index = i + 1;
		memset(buffer, 0, buffer_len);
		memcpy(buffer, &index, sizeof(uint32));
		memcpy(buffer + sizeof(uint32), array_ptr, array_elem_size);
		write(fd, buffer, buffer_len);
	}

	close(fd);
	free(buffer); buffer = NULL;

	return ret_val;
}

uint32 ds_extend_dataset_with_char(char* filename, uint32 add_block_num, char ch){
	const char* THIS_FUNC_NAME = "ds_extend_dataset_with_char";
	uint32 ret_val = 0;
	int fd = -1;
	int open_flags;
	mode_t file_perms;
	int i = 0, j = 0;
	int index = 0;
	char* buffer = NULL;
	int orig_block_size = 0;
	DS_HEADER ds_hdr = {{0}, 0, 0};
	int block_written = 0;

	check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
	add_block_num <= 0 ? debug_print(THIS_FUNC_NAME, "add_block_num cannot be <= 0") : nop();

	open_flags = O_RDWR;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	fd = open(filename, open_flags, file_perms);
	if(fd < 0){
		debug_print(THIS_FUNC_NAME, "failed to open file");
		ret_val = RETCODE_FAILED_TO_OPEN_FILE;
		return ret_val;
	}

	// Verifying the original ds file
	if(!ds_verify_ds(filename, &ds_hdr)){
		debug_print(THIS_FUNC_NAME, "failed to verify the original ds file");
		return RETCODE_FAILED_TO_VRFY_DS;
	}

	buffer = (char*) malloc (ds_hdr.m_ds_block_size);
	if(!buffer){
		debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
		ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
		return ret_val;
	}

	// Appending new blocks to the original ds file
	lseek(fd, 0, SEEK_END);
	for(i = 0; i < add_block_num; i++){
		index = UNASSIGNED_INDEX;	// the added block's index is default to UNASSIGNED_INDEX
		memset(buffer, 0, ds_hdr.m_ds_block_size);
		memcpy(buffer, &index, sizeof(uint32));
		for(j = 0; j < ds_hdr.m_ds_block_size - sizeof(uint32); j++){	// ds_hdr.m_ds_block_size - sizeof(uint32) refers to the actual value length
			*(buffer + sizeof(uint32) + j) = ch;
		}
		write(fd, buffer, ds_hdr.m_ds_block_size);
		block_written ++;
	}
	free(buffer); buffer = NULL;

	return block_written + ds_hdr.m_ds_block_num;
}

uint32 ds_extend_dataset_with_zero(char* filename, uint32 add_block_num){
	return ds_extend_dataset_with_char(filename, add_block_num, 0);
}

uint32 ds_get_ds_block_num(PDS_HEADER pds_hdr){
	const char* THIS_FUNC_NAME = "ds_get_ds_block_num";

	check_pointer_ex(pds_hdr, "pds_hdr", THIS_FUNC_NAME, "null ds header structure pointer");

	return pds_hdr->m_ds_block_num;
}

uint32 ds_get_ds_block_size(PDS_HEADER pds_hdr){
	const char* THIS_FUNC_NAME = "ds_get_ds_block_size";

	check_pointer_ex(pds_hdr, "pds_hdr", THIS_FUNC_NAME, "null ds header structure pointer");

	return pds_hdr->m_ds_block_size;
}

bool ds_verify_ds(char* filename, PDS_HEADER pds_hdr){
	const char* THIS_FUNC_NAME = "ds_verify_ds";
	int fd = -1;
	uint32 bytes_read = 0;
	char ds_version[DS_VERSION_LEN] = {0};
	uint32 ds_block_len = 0;
	uint32 ds_block_num = 0;
	char* ds_blk_buffer = NULL;

	check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
	check_pointer_ex(pds_hdr, "pds_hdr", THIS_FUNC_NAME, "null ds header structure pointer");

	if((fd = fo_open_mhtfile(filename)) < 0){
		debug_print(THIS_FUNC_NAME, "failed to open file");
		return FALSE;
	}

	bytes_read = read(fd, ds_version, 16);
	if(strncmp(ds_version, DS_VERSION, strlen(DS_VERSION)) != 0){
		debug_print(THIS_FUNC_NAME, "dataset file version info. error");
		return FALSE;
	}
	memset(pds_hdr->ds_ver, 0, DS_VERSION_LEN);
	memcpy(pds_hdr->ds_ver, ds_version, strlen(ds_version));

	bytes_read = read(fd, &ds_block_len, sizeof(int));
	if(ds_block_len <= 0){
		debug_print(THIS_FUNC_NAME, "invalid data block size");
		return FALSE;
	}
	pds_hdr->m_ds_block_size = ds_block_len;

	// scan dataset file to determine block number
	if(!(ds_blk_buffer = (char*)malloc(ds_block_len))){
		debug_print(THIS_FUNC_NAME, "failed to allocate ds_blk_buffer");
		return FALSE;
	}
	while(bytes_read = read(fd, ds_blk_buffer, ds_block_len)){
		ds_block_num++;
	}

	if(ds_block_num <= 0){
		debug_print(THIS_FUNC_NAME, "invalid data block number");
		return FALSE;
	}
	pds_hdr->m_ds_block_num = ds_block_num;
	fo_close_mhtfile(fd);

	return TRUE;
}