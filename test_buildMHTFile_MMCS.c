#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"

int main(int argc, char const *argv[])
{
	buildMHTFile_fv("./EVAL_DS_500000.ds", "./OUTPUT_MHT_500000.mht");

	return 0;
}