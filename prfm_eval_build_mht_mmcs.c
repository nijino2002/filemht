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

int main(int argc, char const *argv[])
{
    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer;
 
    if(argc < 2){
        printf("Usage: %s [in-data file name]]\n", argv[0]);
        return 0;
    }

    gettimeofday(&start,NULL);
    buildMHTFile_fv(argv[1]);
    gettimeofday(&end,NULL);
    timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    printf("TIMER = %ld us\n",timer);

    return 0;
}