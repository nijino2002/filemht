#ifndef _DEFS
#define _DEFS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// #define PRINT_INFO_ENABLED

#if (defined __MINGW32__) || (defined __MINGW64__)
    #include <windows.h>
    #define bool BOOL
    #define fsync(fd) (FlushFileBuffers ((HANDLE) _get_osfhandle(fd)) ? 0 : -1)
#endif


#define MAX_SIGNED_INT  0x7fffffff
#define MAX_UNSIGNED_INT    0xffffffff
#define UNASSIGNED_INDEX    MAX_SIGNED_INT
#define HASH_LEN	32
#define HASH_STR_LEN    70
#define ZERO_STR	"00000000000000000000000000000000"	// 32 bytes without considering '\0'
#define RESERVED_CHAR	'R'
#define MHT_HEADER_LEN	128
#define MHT_BLOCK_SIZE		70
#define MHT_HEADER_RSVD_SIZE	104
#define MHT_BLOCK_RSVD_SIZE	    4
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
#define UNASSIGNED_OFFSET   0
#define UNASSIGNED_PAGENO	MAX_SIGNED_INT
#define SINGLENODECMB_PAGENO	-2
#define NODELEVEL_LEAF		0
#define MHT_FILE_MAGIC_STRING       "mhtfile_v1.0"
#define MHT_FILE_MAGIC_STRING_LEN       16
#define MHT_DEFAULT_FILE_NAME		"./mhtfile.mf"
#define MHT_INVALID_FILE_DSCPT      -1      // invalid file descriptor value
#define ASCII_A_POS     65
#define ASCII_Z_POS     90

#define MHT_TMP_FILE_NAME		"./mhtfile_temp.mf"
#define BUF_LEN 4096

#define TEST_STR1	"AAAAA"
#define TEST_STR2	"RRRRRRRR"

typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef unsigned char uchar;

#if (!defined __MINGW32__) && (!defined __MINGW64__)
    typedef enum {FALSE, TRUE} bool;
#endif

extern const uchar g_zeroHash[HASH_LEN];	// firstly defined in defs.c
extern const int g_MhtAttribOffsetArray[MHT_BLOCK_ATRRIB_NUM];		// firstly defined in defs.c
extern uint32 g_mhtFileRootNodeOffset;		// firstly defined in defs.c
extern uint32 g_mhtFirstSplymtLeafOffset;		// firstly defined in defs.c
extern bool g_isEncounterFSLO;      // firstly defined in defs.c
extern int g_mhtFileFD;		// firstly defined in defs.c
extern int g_mhtFileFdRd;      // firstly defined in defs.c

/*
Do nothing.
 */
void nop();

/**
 * @brief      print a '\n'
 */
void println();

/**
 * @brief      Generating a random string with length of str_len
 *
 * @param[in]  str_len  The string length
 *
 * @return     The generated string (needs to be freed after use)
 */
char* generate_random_string(int str_len);

/**
 * @brief      Determines whether the specified d is an integer power of 2.
 *
 * @param[in]  d     the specified d
 *
 * @return     0 is returned if the specified d is an integer power of 2, other integers otherwise.
 */
uint32 is_power_of_2(int d);

/**
 * @brief      Calculating a minimal integer 
 *  that is the integer power of 2 and larger than n
 *
 * @param[in]  n     { The given integer }
 *
 * @return     { The calculated minimal integer }
 */
uint32 cal_the_least_pow2_to_n(uint32 n);

/*
Checking whether a pointer is NULL.
Parameters:
	ptr: the pointer being checked.
	ptr_name: pointer's name.
 */
void check_pointer(void* ptr, const char *ptr_name);

/**
 * @brief      { Checking whether a pointer is NULL. (An extended version) }
 *
 * @param      ptr       The pointer being checked.
 * @param[in]  ptr_name  The pointer's name.
 * @param[in]  from      where this debug message comes from.
 * @param[in]  dbg_msg   The content of the debug message.
 *
 * @return     { if ptr is null, FALSE will be returned, otherwise TRUE will be returned. }
 */
bool check_pointer_ex(void* ptr, const char *ptr_name, const char *from, const char *dbg_msg);

/*
Printing a debug message.
Parameters:
	from: where this debug message comes from.
	dbg_msg: the content of the debug message.
Return:
	NULL.
*/
void debug_print(const char *from, const char *dbg_msg);

/**
 * Printing a given buffer in heximal.
 * @Author   DiLu
 * @DateTime 2021-11-10T14:25:02+0800
 * @param    buf                      [A pointer to the buffer]
 * @param    buf_len                  [Buffer length]
 */
void print_buffer_in_byte_hex(uchar *buf, uint32 buf_len);

#endif


/****************************************************************
 *                Get/Set Functions for Global Variables
*****************************************************************/
uint32 get_mhtFileRootNodeOffset();

uint32 get_mhtFirstSplymtLeafOffset();

bool get_isEncounterFSLO();

void set_mhtFileRootNodeOffset(uint32 rno);

void set_mhtFirstSplymtLeafOffset(uint32 fslo);

void set_isEncounterFSLO(bool is_enc_fslo);