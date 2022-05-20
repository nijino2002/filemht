/**
 * @defgroup   Performance Evaluations
 *
 * @brief      This file contains some definitions used in performance evaluations.
 *
 * @author     Lu Di
 * @date       2022.05.20
 */

#ifndef _PRFM_EVAL_DEFS_H
#define _PRFM_EVAL_DEFS_H

#define DS_ARRAY_LEN 12

const char DATASET_FILENAME_ARRAY[DS_ARRAY_LEN][32] = {"EVAL_DS_100.ds",
										  "EVAL_DS_200.ds",
										  "EVAL_DS_500.ds",
										  "EVAL_DS_1000.ds",
										  "EVAL_DS_2000.ds",
										  "EVAL_DS_5000.ds",
										  "EVAL_DS_10000.ds",
										  "EVAL_DS_20000.ds",
										  "EVAL_DS_50000.ds",
										  "EVAL_DS_100000.ds",
										  "EVAL_DS_200000.ds",
										  "EVAL_DS_500000.ds"};

const char OUTPUT_MHT_FILENAME_ARRAY[DS_ARRAY_LEN][32] = {"OUTPUT_MHT_100.mht",
										  "OUTPUT_MHT_200.mht",
										  "OUTPUT_MHT_500.mht",
										  "OUTPUT_MHT_1000.mht",
										  "OUTPUT_MHT_2000.mht",
										  "OUTPUT_MHT_5000.mht",
										  "OUTPUT_MHT_10000.mht",
										  "OUTPUT_MHT_20000.mht",
										  "OUTPUT_MHT_50000.mht",
										  "OUTPUT_MHT_100000.mht",
										  "OUTPUT_MHT_200000.mht",
										  "OUTPUT_MHT_500000.mht"};

const uint32 DATA_BLOCK_NUM_ARRAY[DS_ARRAY_LEN] = {100, 200, 500, 
												   1000, 2000, 5000, 
												   10000, 20000, 50000, 
												   100000, 200000, 500000};

const uint32 STRING_LENGTH = 32;	// 32-byte string

#endif
