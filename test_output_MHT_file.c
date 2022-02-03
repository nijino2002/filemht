#include "mhtfile_ex.h"

int main(int argc, char const *argv[])
{
	int fd = -1;

	if(argc < 2){
		printf("Usage: %s [MHT filename]\n", argv[0]);
		return 1;
	}

	if((fd = fo_open_mhtfile(argv[1])) < 0){
		printf("Opening %s failed.\n", argv[1]);
		return 1;
	}

	fo_printMHTFile(fd);

	fo_close_mhtfile(fd);

	return 0;
}