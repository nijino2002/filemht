#include "defs.h"
#include "dataelem.h"

/****************************************************************
 * BEGIN          Data  Element: Definition & Operations
*****************************************************************/

void de_init(PDATA_ELEM pelem){
	if(!check_pointer_ex(pelem, "pelem", "de_init", "null pelem"))
		return;

	pelem->m_index = UNASSIGNED_INDEX;
	pelem->m_pdata = NULL;
	pelem->m_data_len = 0;

	return;
}

PDATA_ELEM de_create(int idx, void *d, uint32 d_len){
	PDATA_ELEM pelem = NULL;

	if(!check_pointer_ex(d, "d", "de_create", "null d"))
		return pelem;

	if(d_len <= 0){
		debug_print("de_create", "invalid d_len");
		return pelem;
	}

	pelem = (PDATA_ELEM) malloc (sizeof(DATA_ELEM));
	if(!check_pointer_ex(pelem, "pelem", "de_create", "allocating memory for pelem failed"))
		return pelem;

	de_init(pelem);
	pelem->m_index = idx;
	pelem->m_pdata = d;
	pelem->m_data_len = d_len;

	return pelem;
}

void de_free(PDATA_ELEM pelem, free_func pfree){
	if(!pelem){
		pelem->m_pdata != NULL ? pfree(pelem->m_pdata) : nop();
		free(pelem);
		pelem = NULL;
	}

	return;
}

/****************************************************************
 * END         Data  Element: Definition & Operations
*****************************************************************/