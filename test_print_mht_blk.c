#include "mhtfile.h"

int main(int argc, char const *argv[])
{
	char *filename = "./mhtfile.mf";
	int fd = 0;

	fd = fo_open_mhtfile(filename);
	if(fd < 0) {
		debug_print("main", "open ./mhtfile.mf failed");
		exit(1);
	}

	fo_locate_mht_pos(fd, MHT_HEADER_LEN, SEEK_SET);
	fo_print_mht_block(fd, SEEK_CUR);

	return 0;
}