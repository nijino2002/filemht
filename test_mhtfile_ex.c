#include "mhtfile_ex.h"

int main(int argc, char const *argv[])
{
	PQNode pQHdr = NULL;
	PQNode pQTail = NULL;
	DATA_ELEM de_ary[100];
	int i = 0;

	for(int i; i < 100; i ++){
		de_init(&de_ary[i]);
		de_ary[i].m_index = i + 1;
	}

	process_all_elem(0, &pQHdr, &pQTail, de_ary, 100);

	return 0;
}