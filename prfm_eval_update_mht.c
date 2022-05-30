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

#define     TEST_ROUND    100

int main(int argc, char const *argv[])
{
    int mht_fd = -1;
    int i = 0;
    int found_offset = -1;
    int picked_index = UNASSIGNED_PAGENO;
    char* new_string = NULL;
    char* new_hash = NULL;
    int success_count = 0;
    int choice = 0;

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
    new_hash = (char*) malloc (HASH_LEN);

    for (i = 0; i < TEST_ROUND; ++i)
    {
        picked_index = (rand() % DATA_BLOCK_NUM_ARRAY[choice]) + 1;
        // printf("Picked Index: %d\n", picked_index);
        new_string = generate_random_string(HASH_LEN);
        memset(new_hash, 0, HASH_LEN);
        generateHashByBuffer_SHA256(new_string, HASH_LEN, new_hash, HASH_LEN);
        
        gettimeofday(&start,NULL);
        found_offset = updateMHTBlockHashByPageNo(picked_index, new_hash, HASH_LEN, mht_fd);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

        if(found_offset > 0){
            success_count++;
            acc_timer += timer;
        }

        free(new_string); new_string = NULL;
        lseek(mht_fd, 0, SEEK_SET);
        //println();
    }

    printf("The average update time: %Lg us.\n", (long double)acc_timer / (long double)success_count);
    fo_close_mhtfile(mht_fd);
    free(new_hash);
    if(new_string) free(new_string);

    return 0;
}