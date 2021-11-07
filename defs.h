#ifndef _DEFS
#define _DEFS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_LEN	32
#define ZERO_STR	"00000000000000000000000000000000"	// 32 bytes without considering '\0'
#define MHT_HEADER_LEN	128
#define MHT_CNB_LEN		70
#define MHT_BLOCK_SIZE		66
#define MHT_HEADER_RSVD_SIZE	62
#define MHT_CNB_RSVD_SIZE	4
#define MHT_BLOCK_OFFSET_PAGENO		0
#define MHT_BLOCK_OFFSET_LEVEL		sizeof(int)
#define MHT_BLOCK_OFFSET_HASH		(MHT_BLOCK_OFFSET_LEVEL + sizeof(int))
#define MHT_BLOCK_OFFSET_ISN		(MHT_BLOCK_OFFSET_HASH + HASH_LEN)
#define MHT_BLOCK_OFFSET_IZN		(MHT_BLOCK_OFFSET_ISN + sizeof(char))
#define MHT_BLOCK_OFFSET_LCPN		(MHT_BLOCK_OFFSET_IZN + sizeof(char))
#define MHT_BLOCK_OFFSET_LCOS		(MHT_BLOCK_OFFSET_LCPN + sizeof(int))
#define MHT_BLOCK_OFFSET_RCPN		(MHT_BLOCK_OFFSET_LCOS + sizeof(int))
#define MHT_BLOCK_OFFSET_RCOS		(MHT_BLOCK_OFFSET_RCPN + sizeof(int))
#define MHT_BLOCK_OFFSET_PPN		(MHT_BLOCK_OFFSET_RCOS + sizeof(int))
#define MHT_BLOCK_OFFSET_POS		(MHT_BLOCK_OFFSET_PPN + sizeof(int))
#define MHT_BLOCK_OFFSET_RSVD		(MHT_BLOCK_OFFSET_POS + sizeof(int))
#define MHT_BLOCK_ATRRIB_NUM		12
#define UNASSIGNED_PAGENO	-1
#define SINGLENODECMB_PAGENO	-2
#define NODELEVEL_LEAF		0

#define TEST_STR1	"AAAAA"
#define TEST_STR2	"RRRRRRRR"

typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef unsigned char uchar;
typedef enum {FALSE, TRUE} bool;

extern const uchar g_zeroHash[HASH_LEN];
extern const int g_MhtAttribOffsetArray[MHT_BLOCK_ATRRIB_NUM];

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