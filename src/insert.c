#include <stdio.h>
#include <stdlib.h>

#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"

int	Insert(PFILA2 pfila, TCB_t *tcb) {
	
	// // pfile vazia?
	// if (FirstFila2(pfila)==0) {
	// 	do {
	// 		tcb_it = (TCB_t *) GetAtIteratorFila2(pfila);
	// 		if (tcb->prio < tcb_it->prio) {
	// 			return InsertBeforeIteratorFila2(pfila, tcb);
	// 		}
	// 	} while (NextFila2(pfila)==0);
	// pfile vazia?
	if (LastFila2(pfila)==0) {
		return InsertAfterIteratorFila2(pfila, tcb);
	}	
	return AppendFila2(pfila, (void *)tcb);
}


