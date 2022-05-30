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
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"
#include "mhtfile_ex.h"

#define UTIL_OPT_CMD    "-c"

#define UTIL_CMD_BLOCK_NUM  "n"
#define UTIL_CMD_FSLO_INFO  "f"

int mhtf_util_get_block_num(char* mht_filename, int flag);
int mhtf_util_get_header_info(char* mht_filename, int flag);

int main(int argc, char const *argv[])
{
    int flag = NODELEVEL_LEAF;

    if(argc < 5) {
        printf("Usage: %s [OPTIONS] [CMD_CODE] [CMD_PARAM] [MHT file name]\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], UTIL_OPT_CMD) == 0){
        if(strcmp(argv[2], UTIL_CMD_BLOCK_NUM) == 0){
            mhtf_util_get_block_num((char*)argv[4], atoi(argv[3]));
        }
        else if(strcmp(argv[2], UTIL_CMD_FSLO_INFO) == 0){
            mhtf_util_get_header_info((char*)argv[4], atoi(argv[3]));
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