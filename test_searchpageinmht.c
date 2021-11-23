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
	int pageNo = UNASSIGNED_PAGENO;
	PMHT_BLOCK mhtblk_ptr = NULL;

	if(initOpenMHTFileWR(MHT_DEFAULT_FILE_NAME) < 2){
		printf("Failed to open file %s\n", MHT_DEFAULT_FILE_NAME);
		exit(0);
	}

	printf("Input page number to be searched: ");
	scanf("%d", &pageNo);
	mhtblk_ptr = searchPageByNo(pageNo);
	if(mhtblk_ptr){
		printf("Found page: %d\n", mhtblk_ptr->m_pageNo);
		freeMHTBlock(&mhtblk_ptr);
	}
	else{
		printf("No page found.\n");
	}
	
	return 0;
}