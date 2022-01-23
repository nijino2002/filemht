#include "mhtfile_ex.h"

int main(int argc, char const *argv[])
{
	PQNode pQHdr = NULL;
	PQNode pQTail = NULL;
	PDATA_ELEM de_ary = NULL;
	int i = 0;
	int n = 128;

	de_ary = (PDATA_ELEM) malloc(sizeof(DATA_ELEM) * n);
	for(i = 0; i < n; i ++){
		de_init(&de_ary[i]);
		de_ary[i].m_index = i + 1;
	}

	process_all_elem(0, 0, &pQHdr, &pQTail, de_ary, n);

	return 0;
}