/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements performance evaluation on new node insertion (a common version).
 *
 * @author     Lu Di
 * @date       2022.5.27
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

int main(int argc, char const *argv[])
{
    char tmp_hash_buffer[SHA256_BLOCK_SIZE] = {0};
    char* new_rand_str = NULL;
    int i = 0, j = 0;
    int fd = -1;
    int selected_index = 0;
    int is_success = 0;
    int success_count = 0;
    int TEST_ROUND_ARRAY[DS_ARRAY_LEN] = {0};

    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer = 0;
    unsigned long acc_timer = 0;

    // construct TEST_ROUND_ARRAY
    for (i = 0; i < DS_ARRAY_LEN; ++i)
    {
        TEST_ROUND_ARRAY[i] = DATA_BLOCK_ACTUAL_NUM_ARRAY[i] - DATA_BLOCK_NUM_ARRAY[i];
    }

    srand((uint32)time(NULL));

    for (i = 0; i < DS_ARRAY_LEN; ++i)
    {
        for (j = 0; j < 10; ++j)
        {
            fd = initOpenMHTFileWR((char*)OUTPUT_MHT_FILENAME_INSCMN_ARRAY[i]);

            memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
            // selected_index = DATA_BLOCK_NUM_ARRAY[i] + j + 1;
            selected_index = rand() % DATA_BLOCK_ACTUAL_NUM_ARRAY[i] + 1;
            generateHashByPageNo_SHA256(selected_index, tmp_hash_buffer, SHA256_BLOCK_SIZE);

            // Insert the block
            gettimeofday(&start,NULL);
            is_success = insertNewPageDisorder(selected_index, tmp_hash_buffer, SHA256_BLOCK_SIZE, (char*)OUTPUT_MHT_FILENAME_INSCMN_ARRAY[i]);
            gettimeofday(&end,NULL);
            timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

            if(is_success == 0){
                success_count++;
                acc_timer += timer;
            }

            fo_close_mhtfile(fd);
        } // for j

        printf("ROUND %d, DS size: %d, %d new blocks inserted.\n", i+1, DATA_BLOCK_ACTUAL_NUM_ARRAY[i], j);
        printf("The average insertion time: %Lg us\n", (long double)acc_timer / (long double)success_count);
        println();

        success_count = 0;
        acc_timer = 0;
    } // for i

    return 0;
}
