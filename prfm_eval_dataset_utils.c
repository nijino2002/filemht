/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements a utility tool for dataset.
 *
 * @author     Lu Di
 * @date       2022.5.28
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "defs.h"
#include "dataelem.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"
#include "mhtfile_ex.h"
#include "prfm_eval_defs.h"

int get_ds_block_num(char* ds_file_name, int block_size);

int main(int argc, char const *argv[])
{
    int ds_block_num = 0;

    if(argc < 2) {
        printf("Usage: %s [Dataset file name]\n", argv[0]);
        return 1;
    }

    ds_block_num = get_ds_block_num((char*)argv[1], sizeof(int) + HASH_LEN);
    if(ds_block_num <= 0){
        printf("Failed to get DS block number.\n");
        return 2;
    }

    printf("DS block number: %d, \nwhich is power of 2: %s.\nTo be power of 2, %d blocks need to be added.\n", 
        ds_block_num, is_power_of_2(ds_block_num) == 0 ? "TRUE" : "FALSE", cal_the_least_pow2_to_n(ds_block_num)-ds_block_num);
 
    return 0;
}

int get_ds_block_num(char* ds_file_name, int block_size){
    int count = 0;
    int fd = -1;
    char* read_buffer = NULL;
    int read_byte = 0;

    if(!ds_file_name){
        debug_print("get_ds_block_num", "Null DS file name");
        return 0;
    }

    fd = fo_open_mhtfile(ds_file_name);

    if(fd < 3){
        debug_print("get_ds_block_num", "Invalid file handler");
        return 0;
    }

    if(block_size <= 0){
        debug_print("get_ds_block_num", "Invalid block size");
        return 0;
    }

    read_buffer = (char*) malloc (block_size);
    memset(read_buffer, 0, block_size);

    while(read_byte = fo_read_mht_file(fd, read_buffer, block_size, 0, SEEK_CUR) > 0){
        /* do nothing with the buffer */
        count ++;
        memset(read_buffer, 0, block_size);
    }
    free(read_buffer);
    fo_close_mhtfile(fd);

    return count;
}