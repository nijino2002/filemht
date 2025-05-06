#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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

uint32 ds_create_dataset_random_dso(char* filename, uint32 block_size, uint32 block_num){
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
	uint32* shuffled_indices = NULL;

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

	buffer = (char*) malloc(buffer_len);
	if(!buffer){
		debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
		ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
		close(fd);
		return ret_val;
	}

	shuffled_indices = (uint32*) malloc(sizeof(uint32) * block_num);
	if(!shuffled_indices){
		debug_print(THIS_FUNC_NAME, "failed to allocate index array");
		ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
		free(buffer); buffer = NULL;
		close(fd);
		return ret_val;
	}

	// initialize random seed
	srand(time(NULL));

	// initialize and shuffle indices
	for(i = 0; i < block_num; i++) {
		shuffled_indices[i] = i + 1;
	}
	for(i = block_num - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		uint32 tmp = shuffled_indices[i];
		shuffled_indices[i] = shuffled_indices[j];
		shuffled_indices[j] = tmp;
	}

	// write ds version info
	write(fd, DS_VERSION, DS_VERSION_LEN);

	// write ds block length (DL)
	write(fd, &buffer_len, sizeof(int));

	// write blocks in shuffled order
	for(i = 0; i < block_num; i++){
		index = shuffled_indices[i];
		memset(buffer, 0, buffer_len);
		memcpy(buffer, &index, sizeof(uint32));
		gen_str = generate_random_string(block_size);
		memcpy(buffer + sizeof(uint32), gen_str, block_size);
		free(gen_str); gen_str = NULL;
		write(fd, buffer, buffer_len);
	}

	free(shuffled_indices); shuffled_indices = NULL;
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

uint32 ds_extend_dataset_by_array(char* filename, void* array_ptr, uint32 array_length) {
    const char* THIS_FUNC_NAME = "ds_extend_dataset_by_array";
    uint32 ret_val = RETCODE_OK;
    int fd = -1;
    int open_flags;
    mode_t file_perms;
    int i = 0;
    int index = 0;
    char* buffer = NULL;
    int buffer_len = sizeof(int) + sizeof(void*);  // Assuming array elements are pointers
    uint32 last_index = 0;  // To store the last index in the original dataset

    check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
    check_pointer_ex(array_ptr, "array_ptr", THIS_FUNC_NAME, "null array pointer");
    array_length <= 0 ? debug_print(THIS_FUNC_NAME, "array_length cannot be <= 0") : nop();

    // Open the dataset file in read-write mode
    open_flags = O_RDWR;
    file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    fd = open(filename, open_flags, file_perms);
    if (fd < 0) {
        debug_print(THIS_FUNC_NAME, "failed to open file");
        ret_val = RETCODE_FAILED_TO_OPEN_FILE;
        return ret_val;
    }

    // Read the last data block to get the last index
    lseek(fd, -sizeof(uint32), SEEK_END);  // Seek to the end of the file, just before the last block
    if (read(fd, &last_index, sizeof(uint32)) != sizeof(uint32)) {
        debug_print(THIS_FUNC_NAME, "failed to read last index");
        ret_val = RETCODE_FAILED_TO_READ_FILE;
        close(fd);
        return ret_val;
    }

    last_index = last_index > 0 ? last_index : 0;  // Ensure last_index is at least 0

    buffer = (char*)malloc(buffer_len);
    if (!buffer) {
        debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
        ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
        close(fd);
        return ret_val;
    }

    // Seek to the end of the file to extend it
    lseek(fd, 0, SEEK_END);

    // Iterate over the array and write each element to the dataset
    for (i = 0; i < array_length; i++) {
        index = last_index + 1 + i;  // New index starts from the last index of the original dataset
        memset(buffer, 0, buffer_len);
        memcpy(buffer, &index, sizeof(uint32));
        memcpy(buffer + sizeof(uint32), (char*)array_ptr + i * sizeof(void*), sizeof(void*));  // Assume array elements are pointers
        write(fd, buffer, buffer_len);
    }

    free(buffer);
    close(fd);

    return ret_val;
}

uint32 ds_extend_dataset_by_ds(char* filename, char* input_ds) {
    const char* THIS_FUNC_NAME = "ds_extend_dataset_by_ds";
    uint32 ret_val = RETCODE_OK;
    int fd_in = -1, fd_out = -1;
    DS_HEADER ds_hdr;
    uint32 block_count = 0;
    uint32 i = 0;
    uint32 last_index = 0;
    char* buffer = NULL;
    size_t block_size;

    check_pointer_ex((char*)filename, "filename", THIS_FUNC_NAME, "null file name");
    check_pointer_ex((char*)input_ds, "input_ds", THIS_FUNC_NAME, "null input dataset file name");

    // Verify and open the input dataset
    if (!ds_verify_ds(input_ds, &ds_hdr)) {
        debug_print(THIS_FUNC_NAME, "failed to verify the input dataset");
        return RETCODE_FAILED_TO_VRFY_DS;
    }

    block_count = ds_hdr.m_ds_block_num;
    block_size = ds_hdr.m_ds_block_size;

    fd_in = open(input_ds, O_RDONLY);
    if (fd_in < 0) {
        debug_print(THIS_FUNC_NAME, "failed to open input dataset file");
        return RETCODE_FAILED_TO_OPEN_FILE;
    }

    fd_out = open(filename, O_RDWR);
    if (fd_out < 0) {
        debug_print(THIS_FUNC_NAME, "failed to open output dataset file");
        close(fd_in);
        return RETCODE_FAILED_TO_OPEN_FILE;
    }

    // Read the last data block to get the last index in the original dataset
    lseek(fd_out, -sizeof(uint32), SEEK_END);  // Seek to the end of the output file, just before the last block
    if (read(fd_out, &last_index, sizeof(uint32)) != sizeof(uint32)) {
        debug_print(THIS_FUNC_NAME, "failed to read last index from output dataset");
        ret_val = RETCODE_FAILED_TO_READ_FILE;
        goto cleanup;
    }

    last_index = last_index > 0 ? last_index : 0;  // Ensure last_index is at least 0

    // Allocate buffer to hold each block
    buffer = (char*)malloc(block_size);
    if (!buffer) {
        debug_print(THIS_FUNC_NAME, "failed to allocate buffer");
        close(fd_in);
        close(fd_out);
        return RETCODE_FAILED_TO_ALLOC_MEM;
    }

    // Seek to the end of the output file to extend it
    lseek(fd_out, 0, SEEK_END);

    // Read each block from input dataset and write to output dataset with updated index
    for (i = 0; i < block_count; i++) {
        if (read(fd_in, buffer, block_size) != block_size) {
            debug_print(THIS_FUNC_NAME, "failed to read block from input dataset");
            ret_val = RETCODE_FAILED_TO_READ_FILE;
            goto cleanup;
        }

        // Update the index in the block
        uint32* index_ptr = (uint32*)buffer;
        *index_ptr = last_index + 1 + i;  // New index starts from the last index of the output dataset

        // Write the block to the output dataset file
        write(fd_out, buffer, block_size);
    }

cleanup:
    free(buffer);
    close(fd_in);
    close(fd_out);

    return ret_val;
}

uint32 ds_sort_dataset_by_index(const char* input_filename, const char* output_filename){
    const char* THIS_FUNC_NAME = "ds_sort_dataset_by_index";
    int fd_in = -1, fd_out = -1;
    uint32 ret_val = RETCODE_OK;
    DS_HEADER ds_hdr;
    uint32 block_count = 0;
    BLOCK_INFO* blocks = NULL;
    uint32 i;
    char* buffer = NULL;

    // 验证数据集文件，读取头信息
    if (!ds_verify_ds((char*)input_filename, &ds_hdr)) {
        debug_print(THIS_FUNC_NAME, "failed to verify dataset file");
        return RETCODE_FAILED_TO_VRFY_DS;
    }

    block_count = ds_hdr.m_ds_block_num;
    size_t block_size = ds_hdr.m_ds_block_size;

    fd_in = open(input_filename, O_RDONLY);
    if (fd_in < 0) {
        debug_print(THIS_FUNC_NAME, "failed to open input file");
        return RETCODE_FAILED_TO_OPEN_FILE;
    }

    // 跳过头部信息
    lseek(fd_in, DS_VERSION_LEN + DS_BLOCK_SIZE_LEN, SEEK_SET);

    blocks = (BLOCK_INFO*)malloc(sizeof(BLOCK_INFO) * block_count);
    if (!blocks) {
        debug_print(THIS_FUNC_NAME, "failed to allocate BLOCK_INFO array");
        close(fd_in);
        return RETCODE_FAILED_TO_ALLOC_MEM;
    }

    buffer = (char*)malloc(block_size);
    if (!buffer) {
        debug_print(THIS_FUNC_NAME, "failed to allocate block buffer");
        free(blocks);
        close(fd_in);
        return RETCODE_FAILED_TO_ALLOC_MEM;
    }

    // 读取所有数据块
    for (i = 0; i < block_count; ++i) {
        if (read(fd_in, buffer, block_size) != block_size) {
            debug_print(THIS_FUNC_NAME, "failed to read data block");
            ret_val = RETCODE_FAILED_TO_READ_FILE;
            goto cleanup;
        }

        blocks[i].m_block_data = (char*)malloc(block_size);
        if (!blocks[i].m_block_data) {
            debug_print(THIS_FUNC_NAME, "failed to allocate block memory");
            ret_val = RETCODE_FAILED_TO_ALLOC_MEM;
            goto cleanup;
        }

        memcpy(blocks[i].m_block_data, buffer, block_size);
        memcpy(&blocks[i].m_index, buffer, sizeof(int));  // 从块头取出索引
    }

    // 对 BLOCK_INFO 按照 m_index 升序排序
    qsort(blocks, block_count, sizeof(BLOCK_INFO), compare_block_info);

    // 创建新文件
    fd_out = open(output_filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd_out < 0) {
        debug_print(THIS_FUNC_NAME, "failed to open output file");
        ret_val = RETCODE_FAILED_TO_OPEN_FILE;
        goto cleanup;
    }

    // 写头部
    write(fd_out, DS_VERSION, DS_VERSION_LEN);
    write(fd_out, &ds_hdr.m_ds_block_size, sizeof(int));

    // 写入排序后的数据块
    for (i = 0; i < block_count; ++i) {
        write(fd_out, blocks[i].m_block_data, block_size);
    }

cleanup:
    if (fd_in >= 0) close(fd_in);
    if (fd_out >= 0) close(fd_out);
    if (buffer) free(buffer);
    if (blocks) {
        for (i = 0; i < block_count; ++i) {
            if (blocks[i].m_block_data)
                free(blocks[i].m_block_data);
        }
        free(blocks);
    }

    return ret_val;
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

int compare_block_info(const void* a, const void* b){
    const BLOCK_INFO* ba = (const BLOCK_INFO*)a;
    const BLOCK_INFO* bb = (const BLOCK_INFO*)b;
    return ba->m_index - bb->m_index;
}