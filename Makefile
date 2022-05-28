CC_FLAGS = -g
CC = gcc $(CC_FLAGS)
OBJ = defs.o mhtdefs.o dbqueue.o mhtfile.o mhtfile_ex.o sha256.o dataelem.o
LIBS = -lm

all : main test_searchpageinmht test_updatehashinmht test_fileio \
		sha256_test test_mhtfile test_mhtfile_ex test_insertpageinmht \
		test_insertpagedisorder test_print_mht_blk test_output_MHT_file \
		test_file_buffer prfm_eval_gen_dataset prfm_eval_build_mht prfm_eval_build_mht_mmcs \
		test_buildMHTFile_MMCS prfm_eval_data_int_verify prfm_eval_update_mht \
		prfm_eval_insertsp prfm_eval_insertcmn prfm_eval_gen_dataset_for_insertcmn \
		prfm_eval_dataset_utils mhtfile_utils
.PHONY : all

main : main.o $(OBJ)
	$(CC) -o main main.o $(OBJ) $(LIBS)

test_searchpageinmht : test_searchpageinmht.o $(OBJ)
	$(CC) -o test_searchpageinmht test_searchpageinmht.o $(OBJ) $(LIBS)

test_updatehashinmht: test_updatehashinmht.o $(OBJ)
	$(CC) -o test_updatehashinmht test_updatehashinmht.o $(OBJ) $(LIBS)

test_fileio: test_fileio.o $(OBJ)
	$(CC) -o test_fileio test_fileio.o $(OBJ) $(LIBS)

sha256_test: sha256_test.o $(OBJ)
	$(CC) -o sha256_test sha256_test.o $(OBJ) $(LIBS)

test_mhtfile: test_mhtfile.o
	$(CC) -o test_mhtfile test_mhtfile.o $(OBJ) $(LIBS)

test_buildMHTFile_MMCS: test_buildMHTFile_MMCS.o
	$(CC) -o test_buildMHTFile_MMCS test_buildMHTFile_MMCS.o $(OBJ) $(LIBS)

test_mhtfile_ex: test_mhtfile_ex.o
	$(CC) -o test_mhtfile_ex test_mhtfile_ex.o $(OBJ) $(LIBS) $(CC_FLAGS)

test_insertpageinmht: test_insertpageinmht.o
	$(CC) -o test_insertpageinmht test_insertpageinmht.o $(OBJ) $(LIBS)

test_insertpagedisorder: test_insertpagedisorder.o
	$(CC) -o test_insertpagedisorder test_insertpagedisorder.o $(OBJ) $(LIBS)

test_print_mht_blk: test_print_mht_blk.o
	$(CC) -o test_print_mht_blk test_print_mht_blk.o $(OBJ) $(LIBS)

test_output_MHT_file: test_output_MHT_file.o
	$(CC) -o test_output_MHT_file test_output_MHT_file.o $(OBJ) $(LIBS)

test_file_buffer: test_file_buffer.o
	$(CC) -o test_file_buffer test_file_buffer.o $(OBJ) $(LIBS)

prfm_eval_gen_dataset: prfm_eval_gen_dataset.o
	$(CC) -o prfm_eval_gen_dataset prfm_eval_gen_dataset.o $(OBJ) $(LIBS)

prfm_eval_build_mht: prfm_eval_build_mht.o
	$(CC) -o prfm_eval_build_mht prfm_eval_build_mht.o $(OBJ) $(LIBS)

prfm_eval_build_mht_mmcs: prfm_eval_build_mht_mmcs.o
	$(CC) -o prfm_eval_build_mht_mmcs prfm_eval_build_mht_mmcs.o $(OBJ) $(LIBS)

prfm_eval_data_int_verify: prfm_eval_data_int_verify.o
	$(CC) -o prfm_eval_data_int_verify prfm_eval_data_int_verify.o $(OBJ) $(LIBS)

prfm_eval_update_mht: prfm_eval_update_mht.o
	$(CC) -o prfm_eval_update_mht prfm_eval_update_mht.o $(OBJ) $(LIBS)

prfm_eval_insertsp: prfm_eval_insertsp.o
	$(CC) -o prfm_eval_insertsp prfm_eval_insertsp.o $(OBJ) $(LIBS)

prfm_eval_insertcmn: prfm_eval_insertcmn.o
	$(CC) -o prfm_eval_insertcmn prfm_eval_insertcmn.o $(OBJ) $(LIBS)

prfm_eval_gen_dataset_for_insertcmn: prfm_eval_gen_dataset_for_insertcmn.o
	$(CC) -o prfm_eval_gen_dataset_for_insertcmn prfm_eval_gen_dataset_for_insertcmn.o $(OBJ) $(LIBS)

prfm_eval_dataset_utils: prfm_eval_dataset_utils.o
	$(CC) -o prfm_eval_dataset_utils prfm_eval_dataset_utils.o $(OBJ) $(LIBS)

mhtfile_utils: mhtfile_utils.o
	$(CC) -o mhtfile_utils mhtfile_utils.o $(OBJ) $(LIBS)

$(OBJ) : defs.h mhtdefs.h dbqueue.h mhtfile.h mhtfile_ex.h sha256.h dataelem.h

.PHONY : clean
clean : 
	rm -rf testdbfile.db main test_searchpageinmht test_updatehashinmht test_fileio sha256_test test_mhtfile test_mhtfile_ex test_insertpageinmht test_insertpagedisorder \
	main.o test_searchpageinmht.o test_updatehashinmht.o test_fileio.o sha256_test.o test_mhtfile.o test_mhtfile_ex.o test_insertpageinmht.o test_insertpagedisorder.o \
	test_print_mht_blk test_print_mht_blk.o test_output_MHT_file test_output_MHT_file.o test_file_buffer test_file_buffer.o prfm_eval_gen_dataset prfm_eval_gen_dataset.o \
	prfm_eval_build_mht.o prfm_eval_build_mht prfm_eval_build_mht_mmcs.o prfm_eval_build_mht_mmcs test_buildMHTFile_MMCS test_buildMHTFile_MMCS.o prfm_eval_data_int_verify.o \
	prfm_eval_data_int_verify prfm_eval_update_mht.o prfm_eval_update_mht prfm_eval_insertsp.o prfm_eval_insertsp prfm_eval_insertcmn.o prfm_eval_insertcmn \
	prfm_eval_gen_dataset_for_insertcmn.o prfm_eval_gen_dataset_for_insertcmn prfm_eval_dataset_utils.o prfm_eval_dataset_utils \
	mhtfile_utils.o mhtfile_utils $(OBJ)

.PHONY : clean-ds
clean-ds : 
	rm -rf EVAL_DS_100.ds EVAL_DS_200.ds EVAL_DS_500.ds EVAL_DS_1000.ds EVAL_DS_2000.ds EVAL_DS_5000.ds \
	EVAL_DS_10000.ds EVAL_DS_20000.ds EVAL_DS_50000.ds EVAL_DS_100000.ds EVAL_DS_200000.ds EVAL_DS_500000.ds 

.PHONY : clean-ds_inscmn
clean-ds_inscmn : 
	rm -rf EVAL_DS_INSCMN_100.ds EVAL_DS_INSCMN_200.ds EVAL_DS_INSCMN_500.ds \
	EVAL_DS_INSCMN_1000.ds EVAL_DS_INSCMN_2000.ds EVAL_DS_INSCMN_5000.ds \
	EVAL_DS_INSCMN_10000.ds EVAL_DS_INSCMN_20000.ds EVAL_DS_INSCMN_50000.ds \
	EVAL_DS_INSCMN_100000.ds EVAL_DS_INSCMN_200000.ds EVAL_DS_INSCMN_500000.ds 

.PHONY : clean-mhtfile
clean-mhtfile : 
	rm -rf OUTPUT_MHT_100.mht OUTPUT_MHT_200.mht OUTPUT_MHT_500.mht \
	OUTPUT_MHT_1000.mht OUTPUT_MHT_2000.mht OUTPUT_MHT_5000.mht \
	OUTPUT_MHT_10000.mht OUTPUT_MHT_20000.mht OUTPUT_MHT_50000.mht \
	OUTPUT_MHT_100000.mht OUTPUT_MHT_200000.mht OUTPUT_MHT_500000.mht

.PHONY : clean-mhtfile_inscmn
clean-mhtfile_inscmn : 
	rm -rf OUTPUT_MHT_INSCMN_100.mht OUTPUT_MHT_INSCMN_200.mht OUTPUT_INSCMN_MHT_500.mht \
	OUTPUT_MHT_INSCMN_1000.mht OUTPUT_MHT_INSCMN_2000.mht OUTPUT_MHT_INSCMN_5000.mht \
	OUTPUT_MHT_INSCMN_10000.mht OUTPUT_MHT_INSCMN_20000.mht OUTPUT_MHT_INSCMN_50000.mht \
	OUTPUT_MHT_INSCMN_100000.mht OUTPUT_MHT_INSCMN_200000.mht OUTPUT_MHT_INSCMN_500000.mht