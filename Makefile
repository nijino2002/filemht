CC_FLAGS = -g
CC = gcc $(CC_FLAGS)
OBJ = defs.o mhtdefs.o dbqueue.o mhtfile.o mhtfile_ex.o sha256.o
LIBS = -lm

all : main test_searchpageinmht test_updatehashinmht test_fileio \
		sha256_test test_mhtfile test_mhtfile_ex test_insertpageinmht \
		test_insertpagedisorder test_print_mht_blk
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

test_mhtfile_ex: test_mhtfile_ex.o
	$(CC) -o test_mhtfile_ex test_mhtfile_ex.o $(OBJ) $(LIBS) $(CC_FLAGS)

test_insertpageinmht: test_insertpageinmht.o
	$(CC) -o test_insertpageinmht test_insertpageinmht.o $(OBJ) $(LIBS)

test_insertpagedisorder: test_insertpagedisorder.o
	$(CC) -o test_insertpagedisorder test_insertpagedisorder.o $(OBJ) $(LIBS)

test_print_mht_blk: test_print_mht_blk.o
	$(CC) -o test_print_mht_blk test_print_mht_blk.o $(OBJ) $(LIBS)

$(OBJ) : defs.h mhtdefs.h dbqueue.h mhtfile.h mhtfile_ex.h sha256.h

.PHONY : clean
clean : 
	rm -rf testdbfile.db main test_searchpageinmht test_updatehashinmht test_fileio sha256_test test_mhtfile test_mhtfile_ex test_insertpageinmht test_insertpagedisorder \
	main.o test_searchpageinmht.o test_updatehashinmht.o test_fileio.o sha256_test.o test_mhtfile.o test_mhtfile_ex.o test_insertpageinmht.o test_insertpagedisorder.o \
	test_print_mht_blk test_print_mht_blk.o $(OBJ)
