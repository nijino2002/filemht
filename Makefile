CC = gcc
OBJ = defs.o mhtdefs.o dbqueue.o mhtfile.o sha256.o

all : main test_searchpageinmht test_updatehashinmht test_fileio sha256_test
.PHONY : all

main : main.o $(OBJ)
	$(CC) -o main main.o $(OBJ)

test_searchpageinmht : test_searchpageinmht.o $(OBJ)
	$(CC) -o test_searchpageinmht test_searchpageinmht.o $(OBJ)

test_updatehashinmht: test_updatehashinmht.o $(OBJ)
	$(CC) -o test_updatehashinmht test_updatehashinmht.o $(OBJ)

test_fileio: test_fileio.o $(OBJ)
	$(CC) -o test_fileio test_fileio.o $(OBJ)

sha256_test: sha256_test.o $(OBJ)
	$(CC) -o sha256_test sha256_test.o $(OBJ)

$(OBJ) : defs.h mhtdefs.h dbqueue.h mhtfile.h sha256.h

.PHONY : clean
clean : 
	rm -rf testdbfile.db main test_searchpageinmht test_updatehashinmht test_fileio sha256_test \
	main.o test_searchpageinmht.o test_updatehashinmht.o test_fileio.o sha256_test.o $(OBJ)
