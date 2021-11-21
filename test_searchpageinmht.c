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
	if(initOpenMHTFileWR(MHT_DEFAULT_FILE_NAME) < 2){
		printf("Failed to open file %s\n", MHT_DEFAULT_FILE_NAME);
		exit(0);
	}

	searchPageByNo(5);
	
	return 0;
}