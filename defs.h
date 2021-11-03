#ifndef _DEFS
#define _DEFS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_LEN	32
#define ZERO_STR	"00000000000000000000000000000000"	// 32 bytes without considering '\0'
#define MHT_HEADER_LEN	128
#define MHT_CNB_LEN		70
#define UNASSIGNED_PAGENO	-1
#define NODELEVEL_LEAF		0

#define TEST_STR1	"AAAAA"
#define TEST_STR2	"RRRRRRRR"

typedef unsigned int uint32;
typedef unsigned short int unit16;
typedef unsigned char uchar;
typedef enum {FALSE, TRUE} bool;

extern const uchar g_zeroHash[HASH_LEN];

/*
Do nothing.
 */
void nop();

/*
Checking whether a pointer is NULL.
Parameters:
	ptr: the pointer being checked.
	ptr_name: pointer's name.
 */
void check_pointer(void* ptr, const char *ptr_name);

/*
Printing a debug message.
Parameters:
	from: where this debug message comes from.
	dbg_msg: the content of the debug message.
Return:
	NULL.
*/
void debug_print(const char *from, const char *dbg_msg);

#endif