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

int main(int argc, char const *argv[])
{
    int mht_block_num = 0;
    int flag = NODELEVEL_LEAF;

    if(argc < 2) {
        printf("Usage: %s [MHT file name]\n", argv[0]);
        return 1;
    }

    mht_block_num = get_block_num_in_mhtfile_by_filename((char*)argv[1], flag);
    if(mht_block_num <= 0){
        printf("Failed to get MHT block number.\n");
        return 2;
    }

    if(flag == NODELEVEL_LEAF)
        printf("The number of leaf blocks in MHT file is: %d.\n", mht_block_num);
    else if(flag == 1)
        printf("The number of all blocks in MHT file is: %d.\n", mht_block_num);
    else
        printf("Invalid flags.\n");
    
    return 0;
}