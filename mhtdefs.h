#ifndef _MHTDEFS
#define _MHTDEFS

#include "defs.h"

typedef struct  _MHTNode
{
	uint32	m_pageNo;
	char	m_hash[32];
	uint32	m_lchildOffset;
	uint32 	m_rchildOffset;
	uint32	m_parentOffset;
	uint32	m_lchildPageNo;
	uint32	m_rchildPageNo;
	uint32	m_parentPageNo;
} MHTNode, *PMHTNode;

/*
Making an MHT node.
Parameters: 
	pageno: page number.
	d: data string.
Return: a pointer to an MHT node.
 */
PMHTNode makeMHTNode(uint32 pageno, const char d[]);

/*
Freeing a given MHT node.
Parameters:
	node_ptr: a 2-d pointer to the given node.
Return:
	NULL.
*/
void deleteMHTNode(PMHTNode *node_ptr);

/*
Generating a hash by page number with SHA256 algorithm
Parameter:
	page_no [IN]: page number.
	buf [OUT]: buffer holding output hash value.
	buf_len [IN]: the maximal size of given buffer (buf), which must be larger than 32 bytes.
Return:
	NULL.
*/
void generateHashByPageNo_SHA256(uint32 page_no, char *buf, uint32 buf_len);

/*
Generating a hash by combining two given hashes with SHA256 algorithm
Parameter:
	str1 [IN]: the frist hash.
	str2 [IN]: the second hash.
	buf [OUT]: buffer holding output hash value.
	buf_len [IN]: the maximal size of given buffer (buf), which must be larger than 32 bytes.
Return:
	NULL.
*/
void generateCombinedHash_SHA256(char *hash1, char *hash2, char *buf, uint32 buf_len);

#endif