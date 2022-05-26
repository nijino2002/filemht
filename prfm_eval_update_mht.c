/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements performance evaluation on MHT update.
 *
 * @author     Lu Di
 * @date       2022.5.25
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

#define     TEST_ROUND    1000



int main(int argc, char const *argv[])
{
    int mht_fd = -1;
    int i = 0;
    PMHT_BLOCK pmht_block = NULL;
    int picked_index = UNASSIGNED_PAGENO;
    char* new_string = NULL;
    int success_count = 0;

    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer = 0;
    unsigned long acc_timer = 0;

    if (argc < 3)
    {
        printf("Usage: %s [MHT file name] [DS block number (index 0-11)]\n", argv[0]);
        return -1;
    }

    mht_fd = fo_open_mhtfile(argv[1]);
    choice = atoi(argv[2]);
    if(mht_fd < 0){
        debug_print("main", "Invalid file handlers: mht_fd");
        return -1;
    }

    srand((uint32)time(NULL));

    for (i = 0; i < TEST_ROUND; ++i)
    {
        picked_index = (rand() % DATA_BLOCK_NUM_ARRAY[choice]) + 1;
        pmht_block = searchPageByNo(mht_fd, picked_index);
    }

    fo_close_mhtfile(mht_fd);

    return 0;
}