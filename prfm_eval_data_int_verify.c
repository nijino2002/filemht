/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements performance evaluation on data integrity verification.
 *
 * @author     Ld
 * @date       2022
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

#define     TEST_ROUND    100
#define     DS_BLOCK_LEN        36

int ds_block_num[12] = {128, 256, 512, 1024, 2048, 8192, 16384, 32768, 65536, 131072, 262144, 524288};

void de_data_free(void*);

/**
 * @brief      Finds a DS block by a given index.
 *
 * @param[in]  fd     DS file handler
 * @param[in]  index  The index
 *
 * @return     The offset of the block in DS file
 */
int find_ds_block_by_index(int fd, int index);

int main(int argc, char const *argv[])
{
    int ds_fd = -1;
    int mht_fd = -1;
    PMHT_BLOCK pmht_block = NULL;
    int i = 0; 
    int choice = 0;
    int picked_index = UNASSIGNED_PAGENO;
    int success_count = 0;
    PDATA_ELEM pDE = NULL;
    int de_index = UNASSIGNED_PAGENO;
    char* de_pdata = NULL;
    int de_data_len = DS_BLOCK_LEN - sizeof(int);
    char *ds_blk_buf = NULL;
    int bytes_read = 0;
    bool isVrfyPassed = FALSE;

    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer = 0;
    unsigned long acc_timer = 0;

    if(argc < 4){
        printf("Usage: %s [input dataset file name] [the MHT file name] [DS block number (index 0-11)]\n", argv[0]);
        return 0;
    }

    ds_fd = fo_open_mhtfile(argv[1]);
    mht_fd = fo_open_mhtfile(argv[2]);
    choice = atoi(argv[3]);

    if(choice < 0 || choice > 11){
        debug_print("main", "DS block number must be 0-11");
        return -1;
    }

    if(ds_fd < 0 || mht_fd < 0){
        debug_print("main", "Invalid file handlers: ds_fd or mht_fd");
        return -1;
    }

    srand((uint32)time(NULL));

    ds_blk_buf = (char*) malloc(DS_BLOCK_LEN);

	/* Pick 10000 blocks from a dataset file randomly and 
    verify integrity of these 10000 picked blocks in the MHT file */
    for(i = 0; i < TEST_ROUND; i++){
        picked_index = (rand() % ds_block_num[choice]) + 1;
        // printf("Picked index %d\n", picked_index);
        // read a DS block from the DS file
        memset(ds_blk_buf, 0, DS_BLOCK_LEN);
        de_pdata = (char*) malloc (de_data_len);
        memset(de_pdata, 0, de_data_len);

        // If the DS block cannot be found, it means that the given index actually refers to 
        // an extended block with the index of UNASSIGNED_PAGENO.
        find_ds_block_by_index(ds_fd, picked_index);   // file pointer has already moved to the block offset
        bytes_read = read(ds_fd, ds_blk_buf, DS_BLOCK_LEN);
        memcpy((char*)&de_index, (char*)ds_blk_buf, sizeof(int));
        memcpy(de_pdata, ds_blk_buf + sizeof(int), de_data_len);
        pDE = de_create(de_index, de_pdata, de_data_len);

        gettimeofday(&start,NULL);
        isVrfyPassed = verifyHashInMHT(mht_fd, (void*)pDE);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

        if(isVrfyPassed){
            success_count ++;
            acc_timer += timer;
        }

        de_free(pDE, de_data_free);
        lseek(mht_fd, 0, SEEK_SET);
        // println();
    }

    fo_close_mhtfile(ds_fd);
    fo_close_mhtfile(mht_fd);

    printf("The average verification time: %Lg us.\n", (long double)acc_timer / (long double)success_count);

	return 0;
}

void de_data_free(void* d){
    if(d){
        free(d);
        d = NULL;
    }
    return;
}

int find_ds_block_by_index(int fd, int index){
    int dsblk_index = 0;
    int dsblk_offset = -1;
    char dsblk_buffer[DS_BLOCK_LEN] = {0};
    int bytes_read = 0;

    if(fd < 0){
        debug_print("find_ds_block_by_index", "Invalid DS file handler");
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    while((bytes_read = read(fd, dsblk_buffer, DS_BLOCK_LEN)) > 0){
        memcpy(&dsblk_index, dsblk_buffer, sizeof(int));
        if(dsblk_index == index){
            dsblk_offset = lseek(fd, 0, SEEK_CUR) - DS_BLOCK_LEN;
            lseek(fd, -DS_BLOCK_LEN, SEEK_CUR);
            return dsblk_offset;
        }
        memset(dsblk_buffer, 0, DS_BLOCK_LEN);
    }

    return -1;
}