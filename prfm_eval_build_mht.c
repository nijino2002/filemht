/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements the performance evaluation on building MHT in MMSV (Memory-saving) version.
 *
 * @author     Lu Di
 * @date       2022.05.20
 */
#include <sys/time.h>
#include "defs.h"
#include "prfm_eval_defs.h"
#include "mhtfile.h"
#include "mhtfile_ex.h"

unsigned int extend_indata_file(char* indata_file_name,
                        unsigned int data_block_size,
                        unsigned int data_block_num);

int main(int argc, char const *argv[])
{
    uint32 i = 0;
    PQNode pQHdr = NULL;
    PQNode pQTail = NULL;
    uint32 data_block_size = 0;
    uint32 data_block_num = 10;
    uint32 get_data_block_num = 0;
    uint32 string_len = 10;
    uint32 choice = 0;

    struct  timeval  start;
    struct  timeval  end;
    unsigned long timer;

    if(argc < 2){
        printf("Usage: %s [choice]\n", argv[0]);
        return 0;
    }

    choice = atoi(argv[1]);
    if(choice < 0 || choice >= DS_ARRAY_LEN){
        printf("choice should be between 0 to 11.\n");
        return 0;
    }

    data_block_num = DATA_BLOCK_NUM_ARRAY[choice];
    string_len = STRING_LENGTH;

    printf("%d. Building MHT with %d-block dataset...\n", choice+1, DATA_BLOCK_NUM_ARRAY[choice]);
    printf("\t Dataset file name: %s\n", DATASET_FILENAME_ARRAY[choice]);
    data_block_size = sizeof(uint32) + string_len;
    get_data_block_num = scan_mht_file_data_blocks((char*)DATASET_FILENAME_ARRAY[choice], data_block_size);
    printf("Number of data block: %d\n", get_data_block_num);
    printf("Is power of 2: %d\n", is_power_of_2(get_data_block_num));

    if(is_power_of_2(get_data_block_num) != 0){
        extendSupplementaryBlock4MHTFile((char*)DATASET_FILENAME_ARRAY[choice],
                                        data_block_size,
                                        cal_the_least_pow2_to_n(data_block_num) - data_block_num,
                                        extend_indata_file);
    }
    printf("After extension, the number of data block: %d\n", cal_the_least_pow2_to_n(data_block_num));

    gettimeofday(&start,NULL);
    process_all_elem_fv((char*)DATASET_FILENAME_ARRAY[choice],
                        (char*)OUTPUT_MHT_FILENAME_ARRAY[choice],
                        &pQHdr,
                        &pQTail,
                        data_block_size,
                        FALSE);
    gettimeofday(&end,NULL);
    timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;

    printf("\t Finished building MHT file: %s\n\n", OUTPUT_MHT_FILENAME_ARRAY[choice]);
    printf("TIMER = %ld us\n",timer);

    return 0;
}

unsigned int extend_indata_file(char* indata_file_name,
                        unsigned int data_block_size,
                        unsigned int data_block_num){
    int fd = -1;
    int open_flags;
    mode_t file_perms;
    int i = 0;
    int index = 0x7FFFFFFF;
    char* def_str = NULL;
    char* buffer = NULL;
    int buffer_len = data_block_size;

    if(!indata_file_name || data_block_size <= 0 || data_block_num <= 0){
        printf("Invalid parameters.\n");
        return 0;
    }

    open_flags = O_RDWR;
    file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    fd = open(indata_file_name, open_flags, file_perms);
    lseek(fd, 0, SEEK_END);

    srand((uint32)time(NULL));
    def_str = (char*) malloc (data_block_size - sizeof(int));
    memset(def_str, '0', data_block_size - sizeof(int));
    buffer = (char*) malloc (buffer_len);

    for(i = 0; i < data_block_num; i++){
        memset(buffer, 0, buffer_len);
        memcpy(buffer, &index, sizeof(int));
        memcpy(buffer + sizeof(int), def_str, data_block_size - sizeof(int));
        write(fd, buffer, buffer_len);
    }
    free(def_str);
    close(fd);
}