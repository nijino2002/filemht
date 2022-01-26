#include "mhtfile_ex.h"

#define INDATA_FILENAME	"./indata_orig.dat"
#define INDATA_PROC_FILENAME	"./indata_orig.dat"

void generate_indata_file_orig();

int main(int argc, char const *argv[])
{
	int fd_indata_orig = -1;	// original data
	int fd_indata_proc = -1;	// data having been pre-processed
	PQNode pQHdr = NULL;
	PQNode pQTail = NULL;
	PDATA_ELEM de_ary = NULL;
	int i = 0;
	int n = 16;

	de_ary = (PDATA_ELEM) malloc(sizeof(DATA_ELEM) * n);
	for(i = 0; i < n; i ++){
		de_init(&de_ary[i]);
		de_ary[i].m_index = i + 1;
	}

	process_all_elem(0, 0, &pQHdr, &pQTail, de_ary, n);

	return 0;
}

void generate_indata_file_orig(){
	int fd = -1;
	int open_flags;
	mode_t file_perms;

}