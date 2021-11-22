#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"

extern PQNode g_pQHeader;
extern PQNode g_pQ;

int main(int argc, char const *argv[])
{
	PMHT_BLOCK mhtblk_ptr = NULL;

	if(initOpenMHTFileWR(MHT_DEFAULT_FILE_NAME) < 2){
		printf("Failed to open file %s\n", MHT_DEFAULT_FILE_NAME);
		exit(0);
	}

	mhtblk_ptr = searchPageByNo(5555);
	printf("Found page: %d\n", mhtblk_ptr->m_pageNo);
	freeMHTBlock(&mhtblk_ptr);
	
	return 0;
}