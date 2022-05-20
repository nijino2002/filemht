/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements the performance evaluation on building MHT in MMCS version.
 *
 * @author     Lu Di
 * @date       2022.05.20
 */

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

    for (i = 0; i < DS_ARRAY_LEN; i++){
        data_block_num = DATA_BLOCK_NUM_ARRAY[i];
        string_len = STRING_LENGTH;

        printf("%d. Building MHT with %d-block dataset...\n", i+1, DATA_BLOCK_NUM_ARRAY[i]);
        printf("\t Dataset file name: %s\n", DATASET_FILENAME_ARRAY[i]);
        data_block_size = sizeof(uint32) + string_len;
        get_data_block_num = scan_mht_file_data_blocks((char*)DATASET_FILENAME_ARRAY[i], data_block_size);
        printf("Number of data block: %d\n", get_data_block_num);
        printf("Is power of 2: %d\n", is_power_of_2(get_data_block_num));

        if(is_power_of_2(get_data_block_num) != 0){
            extendSupplementaryBlock4MHTFile((char*)DATASET_FILENAME_ARRAY[i],
                                            data_block_size,
                                            cal_the_least_pow2_to_n(data_block_num) - data_block_num,
                                            extend_indata_file);
        }
        printf("After extension, the number of data block: %d\n", cal_the_least_pow2_to_n(data_block_num));

        process_all_elem_fv((char*)DATASET_FILENAME_ARRAY[i],
                            (char*)OUTPUT_MHT_FILENAME_ARRAY[i],
                            &pQHdr,
                            &pQTail,
                            data_block_size,
                            FALSE);

        printf("\t Finished building MHT file: %s\n\n", OUTPUT_MHT_FILENAME_ARRAY[i]);
    } //for

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