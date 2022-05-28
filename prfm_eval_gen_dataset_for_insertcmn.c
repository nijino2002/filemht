/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file implements dataset generation for insertion (the common version) evaluations.
 *
 * @author     Lu Di
 * @date       2022.05.28
 */

#include "defs.h"
#include "prfm_eval_defs.h"
#include "mhtfile.h"

const int MAX_UPPER_BD = 1000000;

void quickSort(int array[], int low, int high);
int *getOrderedArray(int N, int m);
int getStandard(int array[], int low, int high);

/**
 * @brief      Generating a dataset file for performance evaluation.
 *             Each data block in the dataset has the following structure:
 *                 index: an integer from 1 to N
 *                 data: a string in a given length (in byte)
 *
 * @param[in]  file_name       The file name
 * @param[in]  data_block_num  The data block number
 * @param[in]  str_len         The string length
 */     
void gen_ds_file(const char* file_name, int* index_array, uint32 data_block_num, uint32 string_len);

int main(int argc, char const *argv[])
{
    int i = 0, j = 0;
    int* index_array = NULL;

    srand(time(NULL));

    for (i = 0; i < DS_ARRAY_LEN; ++i)
    {
        index_array = getOrderedArray(MAX_UPPER_BD, DATA_BLOCK_NUM_ARRAY[i]);
        gen_ds_file(DATASET_FILENAME_FOR_INSERTCMN_ARRAY[i], 
                    index_array, DATA_BLOCK_NUM_ARRAY[i], STRING_LENGTH);
        free(index_array);
    }

    return 0;
}

/***************** Helper Functions *********************/

void gen_ds_file(const char* file_name, 
                 int* index_array, 
                 uint32 data_block_num,     // also the size of index_array
                 uint32 string_len) {
    int fd = -1;
    int open_flags;
    mode_t file_perms;
    int i = 0;
    int index = 0;
    char* gen_str = NULL;
    char* buffer = NULL;
    int buffer_len = sizeof(int) + string_len;

    check_pointer_ex((char*)file_name, "file_name", "gen_ds_file", "null file name");
    data_block_num <= 0 || string_len <= 0 ? debug_print("gen_ds_file", "neither data_block_num nor string_len can be <= 0") : nop();

    open_flags = O_CREAT | O_WRONLY | O_TRUNC;
    file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    fd = open(file_name, open_flags, file_perms);

    buffer = (char*) malloc (buffer_len);

    for(i = 0; i < data_block_num; i++){
        index = index_array[i];
        memset(buffer, 0, buffer_len);
        memcpy(buffer, &index, sizeof(uint32));
        gen_str = generate_random_string(string_len);
        memcpy(buffer + sizeof(uint32), gen_str, string_len);
        free(gen_str); gen_str = NULL;
        write(fd, buffer, buffer_len);
    }

    close(fd);
}

int *getOrderedArray(int N, int m) {
    int *num = (int *) malloc(4 * m);
    int *set = (int *) malloc(4 * (N + 1));
    //初始化set
    for (int i = 0; i < N + 1; ++i) {
        set[i] = 0;
    }

    int i = 0;
    while (i < m) {
//        srand((unsigned)time(NULL));
        int temp = rand() % N + 1;
        if (set[temp] == 0) {
            num[i] = temp;
            set[temp] = temp;
            i++;
        }
    }

    free(set);
    quickSort(num, 0, m - 1);
    return num;
}

//--------------------------------------------排序start
/*获取基准坐标，并相对有序（左边比基准坐标小，右边比基准坐标大）*/
int getStandard(int array[], int low, int high) {
    int key = array[low];  //临时保存基准元素
    while (low < high) {
        //high指针从后向前遍历 ， 元素比基准元素大则指针向前移动 则比基准元素小则和基准元素交换
        while (low < high && array[high] >= key) {
            high--;
        }
        if (low < high) {
            array[low] = array[high];  //赋值给第一个元素，因为第一个元素作为基准元素已经临时保存了，所可以直接赋值
        }
        //low指针从前向后遍历 ， 元素比基准元素小则指针向后移动 否则比基准元素大则和基准元素交换
        while (low < high && array[low] <= key) {
            low++;
        }
        if (low < high) {
            array[high] = array[low];  //复制给high指针所指得位置，因为在11行已经赋值给array[low]了
        }
    }
    array[low] = key;
    return low;
}

void quickSort(int array[], int low, int high) {
    if (low < high) {  //递归出口
        int standard = getStandard(array, low, high);
        quickSort(array, low, standard - 1);  //比基准元素小的部分继续调用快速排序
        quickSort(array, standard + 1, high);   //比基准元素大的部分继续调用快速排序
    }
}