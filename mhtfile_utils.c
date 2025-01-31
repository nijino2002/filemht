/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements a utility tool for MHT file.
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
#include "ds.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"
#include "mhtfile_ex.h"

#define UTIL_OPT_CMD    "-c"

#define UTIL_CMD_BLOCK_NUM  "n"
#define UTIL_CMD_FSLO_INFO  "f"

#define UTIL_CMD_DS_BLOCK_NUM   "dsn"       // dataset block number
#define UTIL_CMD_DS_CREATE_16      "dsc16"       // create new ds file with block size of 16 bytes and random value

int mhtf_util_get_block_num(char* mht_filename, int flag);
int mhtf_util_get_header_info(char* mht_filename, int flag);

int mhtf_util_ds_create_16(char* ds_filename, int flag);
int mhtf_util_get_ds_block_num(char* ds_filename, int flag);

int main(int argc, char const *argv[])
{
    int flag = NODELEVEL_LEAF;

    if(argc < 5) {
        printf("Usage: %s [OPTIONS] [CMD_CODE] [CMD_PARAM] [MHT/DS file name]\n", argv[0]);
        printf("Instructions:\n");
        printf("1. OPTIONS: currently, only \'-c\' is available, which means we will use command. \n");
        printf("2. CMD_CODE: currently, \'n\' and \'f\' are available.\n");
        printf("\t1) \'n\': show MHT block number. CMD_PARAM==0: show leaf block number; CMD_PARAM==1: show the number of all blocks.\n");
        printf("\t2) \'f\': show MHT header information. CMD_PARAM is unused, any character is accepted.\n");
        printf("3. CMD_PARAM: see the instructions in CMD_CODE\n");
        printf("4. MHT file name: MHT file name with path.\n");
        return 1;
    }

    if(strcmp(argv[1], UTIL_OPT_CMD) == 0){
        if(strcmp(argv[2], UTIL_CMD_BLOCK_NUM) == 0){
            mhtf_util_get_block_num((char*)argv[4], atoi(argv[3]));
        }
        else if(strcmp(argv[2], UTIL_CMD_FSLO_INFO) == 0){
            mhtf_util_get_header_info((char*)argv[4], atoi(argv[3]));
        }
        else if(strcmp(argv[2], UTIL_CMD_DS_BLOCK_NUM) == 0){
            mhtf_util_get_ds_block_num((char*)argv[4], atoi(argv[3]));
        }
        else{
            printf("Bad command or parammeter.\n");
        }
    }
    else{
        printf("Bad options.\n");
    }
    
    return 0;
}


int mhtf_util_get_block_num(char* mht_filename, int flag){
    int bn = 0;

    bn = get_block_num_in_mhtfile_by_filename(mht_filename, flag);
    if(bn <= 0){
        printf("Failed to get MHT block number.\n");
        return 2;
    }

    if(flag == NODELEVEL_LEAF)
        printf("The number of leaf blocks in MHT file is: %d.\n", bn);
    else if(flag == 1)
        printf("The number of all blocks in MHT file is: %d.\n", bn);
    else
        printf("Invalid flags.\n");

    return bn;
}

int mhtf_util_get_header_info(char* mht_filename, int flag){
    const char* THIS_FUNC_NAME = "mhtmhtf_util_get_header_info";
    int fd = -1;
    char read_block_buf[MHT_BLOCK_SIZE]={0};
    char* read_header_buffer = NULL;
    PMHT_FILE_HEADER mhthdr_ptr = NULL;
    int fslo_index = 0;
    int rno_index = 0;

    if(!check_pointer_ex(mht_filename, "mht_filename", THIS_FUNC_NAME, "Null mht_filename")){
        return -1;
    }

    read_header_buffer = (char*) malloc (MHT_HEADER_LEN);
    memset(read_header_buffer, 0, MHT_HEADER_LEN);

    mhthdr_ptr = makeMHTFileHeader();
    fd = fo_open_mhtfile(mht_filename);
    fo_read_mht_file_header(fd, read_header_buffer, MHT_HEADER_LEN);
    unserialize_mht_file_header(read_header_buffer, MHT_HEADER_LEN, &mhthdr_ptr);
    fo_read_mht_file(fd, read_block_buf, MHT_BLOCK_SIZE, mhthdr_ptr->m_firstSupplementaryLeafOffset, SEEK_SET);
    fslo_index = *(int*)read_block_buf;
    fo_read_mht_file(fd, read_block_buf, MHT_BLOCK_SIZE, mhthdr_ptr->m_rootNodeOffset, SEEK_SET);
    rno_index = *(int*)read_block_buf;

    printf("The RNO is: %d bytes. Index is %x.\n", mhthdr_ptr->m_rootNodeOffset, rno_index);
    printf("The FSLO is: %d bytes. Index is %x.\n", mhthdr_ptr->m_firstSupplementaryLeafOffset, fslo_index);

    freeMHTFileHeader(&mhthdr_ptr);
    free(read_header_buffer);

    return 0;
}

/**
 * @brief      { Get the number of data blocks in the input dataset. 
 *               Each dataset has the following file structure:
 *               -------------------------------------------------------------------------------------------------------
 *               | "ds_v1.0" | data length of each block (DL) | data index 1 | data 1 | data index 2 | data 2 | ...... |
 *               -------------------------------------------------------------------------------------------------------
 *               |  16 bytes |         4 bytes                |   4 bytes    |  DL    |     4 bytes  |  DL    | ...... |
 *               -------------------------------------------------------------------------------------------------------
 *               }
 *
 * @param      ds_filename  The ds filename
 * @param[in]  flag         The flag
 *
 * @return     { description_of_the_return_value }
 */
int mhtf_util_get_ds_block_num(char* ds_filename, int flag){
    const char* THIS_FUNC_NAME = "mhtf_util_get_ds_block_num";
    DS_HEADER ds_header = {{0}, 0, 0};

    if(!ds_verify_ds(ds_filename, &ds_header)){
        printf("Failed to verify dataset file.\n");
        return 0;
    }

    printf("Dataset file %s contains %d blocks.\n", ds_filename, ds_header.m_ds_block_num);

    return 0;
}