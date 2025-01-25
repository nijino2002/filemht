/**
 * @defgroup   TEST_MHTFILE_EX test mhtfile ex
 *
 * @brief      This program gives an example on how to create an MHT file 
 * 				based on a randomly generated input data file (abbrv. in-data file).
 *
 * @author     Ld
 * @date       2025
 */

#include "mhtfile_ex.h"

#define INDATA_FILENAME	"./indata_orig.dat"
#define OUT_MHT_FILENAME	"./out_mht_file.mf"

/**
 * @brief      Randomly generating input data file with given number of data blocks.
 * 				The generated data block has the following structure:
 * 				int data_index;
 * 				char* data;		// data length equals to "string_len"
 *
 * @param[in]  data_block_num  The data block number
 * @param[in]  string_len      The data size (in byte) for each block
 */
void generate_indata_file_orig(int data_block_num,
							   int string_len);

/**
 * @brief      Extends the data blocks in input data file so that the number of the total 
 * 				data blocks can satisfy integer power of 2.
 *
 * @param      indata_file_name  The input data file name
 * @param[in]  data_block_size   The data block size
 * @param[in]  data_block_num    The data block number
 *
 * @return     { description_of_the_return_value }
 */
unsigned int extend_indata_file(char* indata_file_name,
						unsigned int data_block_size,
						unsigned int data_block_num);

int main(int argc, char const *argv[])
{
	int fd_indata_orig = -1;	// original data
	int fd_indata_proc = -1;	// data having been pre-processed
	PQNode pQHdr = NULL;
	PQNode pQTail = NULL;
	PDATA_ELEM de_ary = NULL;
	int i = 0;
	int n = 16;
	int data_block_size = 0;
	const int data_block_num = 10;
	int get_data_block_num = 0;
	const int string_len = 10;

	generate_indata_file_orig(data_block_num, string_len);
	data_block_size = sizeof(int) + string_len;
	// Here, get_data_block_num == data_block_num
	get_data_block_num = scan_mht_file_data_blocks(INDATA_FILENAME, data_block_size);
	printf("Number of data block: %d\n", get_data_block_num);
	printf("Is power of 2: %d\n", is_power_of_2(get_data_block_num));
	if(is_power_of_2(get_data_block_num) != 0){
		extendSupplementaryBlock4InDataFile(INDATA_FILENAME,
										data_block_size,
										cal_the_least_pow2_to_n(data_block_num) - data_block_num,
										extend_indata_file);
	}
	printf("After extension, the number of data block: %d\n", cal_the_least_pow2_to_n(data_block_num));

	// Building MHT with the given input data file
	// The output MHT file will be written in the file named OUT_MHT_FILENAME
	process_all_elem_fv(INDATA_FILENAME,
						OUT_MHT_FILENAME,
						&pQHdr,
						&pQTail,
						data_block_size,
						FALSE);


	return 0;
}

void generate_indata_file_orig(int data_block_num,
							   int string_len){
	int fd = -1;
	int open_flags;
	mode_t file_perms;
	int i = 0;
	int index = 0;
	char* gen_str = NULL;
	char* buffer = NULL;
	int buffer_len = sizeof(int) + string_len;

	open_flags = O_CREAT | O_WRONLY | O_TRUNC;
	file_perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

	fd = open(INDATA_FILENAME, open_flags, file_perms);

	srand((uint32)time(NULL));
	buffer = (char*) malloc (buffer_len);

	for(i = 0; i < data_block_num; i++){
		index = i + 1;
		memset(buffer, 0, buffer_len);
		memcpy(buffer, &index, sizeof(int));
		gen_str = generate_random_string(string_len);
		memcpy(buffer + sizeof(int), gen_str, string_len);
		free(gen_str);
		write(fd, buffer, buffer_len);
	}

	close(fd);
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

	return RETCODE_OK;
}