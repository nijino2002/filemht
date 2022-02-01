#include "mhtfile_ex.h"
#include <errno.h>

int main(int argc, char const *argv[])
{
	int fd = -1;
	char* rand_str = NULL;
	char* output_buffer = NULL;
	int ret = 0;

	output_buffer = (char*) malloc (64);
	memset(output_buffer, 0, 64);

	rand_str = generate_random_string(32);
	printf("Randomly generated string: %s\n", rand_str);
	fd = fo_create_mhtfile("./test_flush.dat");
	fo_close_mhtfile(fd);
	fd = fo_open_mhtfile("./test_flush.dat");
	write(fd, rand_str, strlen(rand_str));
	ret = fsync(fd);
	printf("fsync returned %d\n", ret);
	ret = lseek(fd, 0, SEEK_CUR);
	printf("Current file pointer offset: %d\n", ret);
	ret = lseek(fd, -16, SEEK_CUR);
	printf("Current file pointer offset: %d\n", ret);
	ret = read(fd, output_buffer, 16);
	printf("Bytes read: %d\n", ret);
	if(ret < 0){
		printf("Read ERROR: %d - %s\n", errno, strerror(errno));
	}
	printf("Read string: %s\n", output_buffer);

	fo_close_mhtfile(fd);
	free(output_buffer);
	free(rand_str);
	return 0;
}