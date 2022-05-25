/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements the performance evaluation on building MHT in MMCS (Memory-consuming) version.
 *
 * @author     Lu Di
 * @date       2022.05.20
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"
#include "prfm_eval_defs.h"

int main(int argc, char const *argv[])
{
    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer;
    int choice = 0;
 
    if(argc < 3){
        printf("Usage: %s [in-data file name] [output MHT file name index (0-11)]\n", argv[0]);
        return 0;
    }

    choice = atoi(argv[2]);
    if(choice < 0 || choice >= DS_ARRAY_LEN){
        printf("MHT file name index must be in range [0-11].\n");
        return -1;
    }

    gettimeofday(&start,NULL);
    buildMHTFile_fv(argv[1], OUTPUT_MHT_FILENAME_ARRAY[choice]);
    gettimeofday(&end,NULL);
    timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    printf("TIMER = %ld us\n",timer);

    return 0;
}