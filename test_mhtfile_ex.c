#include "mhtfile_ex.h"

#define INDATA_FILENAME	"./indata_orig.dat"
#define INDATA_EXT_FILENAME	"./indata_ext_orig.dat"

void generate_indata_file_orig(int data_block_num,
							   int string_len);
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
	get_data_block_num = scan_mht_file_data_blocks(INDATA_FILENAME, data_block_size);
	printf("Number of data block: %d\n", get_data_block_num);
	printf("Is power of 2: %d\n", is_power_of_2(get_data_block_num));
	if(is_power_of_2(get_data_block_num) != 0){
		extendSupplementaryBlock4MHTFile(INDATA_FILENAME,
										data_block_size,
										16-data_block_num,
										extend_indata_file);
	}

	de_ary = (PDATA_ELEM) malloc(sizeof(DATA_ELEM) * n);
	for(i = 0; i < n; i ++){
		de_init(&de_ary[i]);
		de_ary[i].m_index = i + 1;
	}

	process_all_elem(0, 0, &pQHdr, &pQTail, de_ary, n);

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
}