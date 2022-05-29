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

int mhtf_util_get_block_num(char* mht_filename, int flag);

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