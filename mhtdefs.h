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

#endif