/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements performance evaluation on new node insertion (a special version).
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

#define TEST_ROUND 10

int main(int argc, char const *argv[])
{
    int index = UNASSIGNED_PAGENO;
    PMHT_BLOCK mhtblk_ptr = NULL;
    PMHTNode mhtnode_ptr = NULL;
    PQNode qnode_ptr = NULL;
    PMHT_FILE_HEADER mht_filehdr_ptr = NULL;
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
        for (j = 0; j < TEST_ROUND; ++j)
        {
            fd = initOpenMHTFileWR((char*)OUTPUT_MHT_FILENAME_ARRAY[i]);

            // Make a new MHT block
            new_rand_str = generate_random_string(HASH_LEN);
            memset(tmp_hash_buffer, 0, SHA256_BLOCK_SIZE);
            selected_index = DATA_BLOCK_NUM_ARRAY[i] + j + 1;
            generateHashByPageNo_SHA256(selected_index, tmp_hash_buffer, SHA256_BLOCK_SIZE);
            mhtnode_ptr = makeMHTNode(selected_index, tmp_hash_buffer);
            qnode_ptr = makeQNode(mhtnode_ptr, NODELEVEL_LEAF);
            mhtblk_ptr = makeMHTBlock();
            convert_qnode_to_mht_block(qnode_ptr, &mhtblk_ptr);

            //Read the MHT file header
            if(!(mht_filehdr_ptr = readMHTFileHeader(fd)))
            {
                debug_print("main", "Failed to read MHT file header");
                return -1;
            }
            // Output the MHT file header, especially the first supplementary block (MHT_FILE_HEADER.m_firstSupplementaryLeafOffset)
            // printf("The offset of the first supplementary block is: %d\n", mht_filehdr_ptr->m_firstSupplementaryLeafOffset);

            // Insert the block
            gettimeofday(&start,NULL);
            is_success = insertNewMHTBlock(mhtblk_ptr, fd);
            gettimeofday(&end,NULL);
            timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

            if(is_success == 0){
                success_count++;
                acc_timer += timer;
            }


            free(new_rand_str); new_rand_str = NULL;
            free(mhtnode_ptr); mhtnode_ptr = NULL;
            free(qnode_ptr); qnode_ptr = NULL;
            free(mhtblk_ptr); mhtblk_ptr = NULL;
            free(mht_filehdr_ptr); mht_filehdr_ptr = NULL;
            fo_close_mhtfile(fd);
        } // for j

        printf("ROUND %d, DS size: %d, %d new blocks inserted.\n", i+1, DATA_BLOCK_ACTUAL_NUM_ARRAY[i], j);
        printf("The average insertion time: %Lg us\n", (long double)acc_timer / (long double)success_count);
        println();

        success_count = 0;
        acc_timer = 0;
    } // for i
    
    if(new_rand_str) {free(new_rand_str); new_rand_str = NULL;}
    if(mhtnode_ptr) {free(mhtnode_ptr); mhtnode_ptr = NULL;}
    if(qnode_ptr) {free(qnode_ptr); qnode_ptr = NULL;}
    if(mhtblk_ptr) {free(mhtblk_ptr); mhtblk_ptr = NULL;}
    if(mht_filehdr_ptr) {free(mht_filehdr_ptr); mht_filehdr_ptr = NULL;}

    return 0;
}