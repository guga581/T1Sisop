#
# Makefile para programas exemplo
# 


CC=gcc
LIB_DIR=../lib
FLAGS=-m32 -o
LIB=-lcthread

all: exemplo mandel series teste_vetor

exemplo:
	$(CC) $(FLAGS) exemplo exemplo.c -L$(LIB_DIR) $(LIB)

mandel:
	$(CC) $(FLAGS) mandel mandel.c -L$(LIB_DIR) $(LIB)

series:
	$(CC) $(FLAGS) series series.c -L$(LIB_DIR) $(LIB)

teste_vetor:
	$(CC) $(FLAGS) teste_vetor teste_vetor.c -L$(LIB_DIR) $(LIB)

clean:
	find -type f ! -name '*.c' ! -name 'Makefile' -delete