#include "defs.h"

void nop() {
	return;
}

void check_pointer(void* ptr, const char *ptr_name) {
	if(!ptr){
		printf("Pointer %s is NULL.\n", ptr_name);
	}

	return;
}