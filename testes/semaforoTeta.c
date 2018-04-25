/**
** Teste da função ccreate
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"

int id0, id1, id2;
csem_t sem1, sem2, sem3;

void* func0() {
	printf("Eu sou a thread ID1 \n");
	cwait(&sem1);
	csignal(&sem1);

}

void* func1() {
	printf("Eu sou a thread ID2 \n");
	cwait(&sem2);
	csignal(&sem2);

}

void* func2() {
	printf("Eu sou a thread ID3 \n");
	cwait(&sem3);
	csignal(&sem3);

}

int main(int argc, char *argv[]) {
	

	id0 = ccreate(func0, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id0);
	
	id1 = ccreate(func1, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id1);

	id2 = ccreate(func2, (void *) NULL, 0);
	printf("Eu sou a thread de TID: %d\n", id2);
	

	printf("Eu sou a main após a criação de threads\n");


	if (csem_init(&sem1, 1) != 0){
		printf("Nao criou semaforo 1\n");
		return -1;
	}

	if (csem_init(&sem2, 1) != 0){
		printf("Nao criou semaforo 2\n");
		return -1;
	}

	if (csem_init(&sem3, 1) != 0){
		printf("Nao criou semaforo 3\n");
		return -1;
	}

	if (cjoin(id0) != 0){
		printf("Erro no cjoin\n");
		return -1;
	}

	printf("Sou a main voltando\n");

	return 0;
}
