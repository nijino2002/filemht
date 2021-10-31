#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mhtdefs.h"
#include "dbqueue.h"
#include "mhtfile.h"

/****** global variable definitions ******/
PQNode g_pQHeader = NULL;	// pointer to queue's header
PQNode g_pQ = NULL;			// pointer to queue's tail (current element)

/*
Testing functions of queue
 */
void test_queue();

void test_queue() {
	const char str[32] = TEST_STR1;
	int i = 0;
	PQNode qnode_ptr = NULL;
	PMHTNode mhtnode_ptr = NULL;

	if(g_pQHeader != NULL && g_pQ != NULL)
		freeQueue(&g_pQHeader, &g_pQ);
	else if(g_pQHeader){
		freeQueue2(&g_pQHeader);
	}
	else if(g_pQ){
		freeQueue3(&g_pQ);
	}
	else{	// both of g_pQHeader and g_pQ are NULL
		;	// do nothing
	}

	initQueue(&g_pQHeader, &g_pQ);
	check_pointer((void*)g_pQHeader, "g_pQHeader");
	check_pointer((void*)g_pQ, "g_pQ");
	for(i = 0; i < 100; i++){
		mhtnode_ptr = makeMHTNode(i+1, str); check_pointer((void*)mhtnode_ptr, "mhtnode_ptr");
		qnode_ptr = makeQNode(mhtnode_ptr, 0); check_pointer((void*)qnode_ptr, "qnode_ptr");
		enqueue(&g_pQHeader, &g_pQ, qnode_ptr);
	}
	printf("%d\n", g_pQHeader->m_length);

	//output
	while(qnode_ptr = dequeue(&g_pQHeader, &g_pQ)){
		check_pointer((void*)qnode_ptr, "qnode_ptr");
		printf("Node page number: %d, hash value: %s, level: %d\n", 
			qnode_ptr->m_MHTNode_ptr->m_pageNo,
			qnode_ptr->m_MHTNode_ptr->m_hash,
			qnode_ptr->m_level);
		deleteQNode(&qnode_ptr);
	}
	printf("%d\n", g_pQHeader->m_length);
	freeQueue(&g_pQHeader, &g_pQ);
}

int main(int argc, char const *argv[])
{
	char hash1[40] = {0};
	char hash2[40] = {0};
	char combinedHash[40] = {0};
	char hash_string[65] = {0};
	testMHTQueue();
	generateHashByPageNo_SHA256(25, hash1, 40);
	generateHashByPageNo_SHA256(26, hash2, 40);
	generateCombinedHash_SHA256(hash1, hash2, combinedHash, 40);
	print_hash_value(combinedHash);
	convert_hash_to_string(combinedHash, hash_string, 65);
	printf("%s\n", hash_string);
	return 0;
}