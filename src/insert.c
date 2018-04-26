

/********************************************************************
	Vers. 17.2 - 11/09/2017
********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"

int	Insert(PFILA2 pfila, TCB_t *tcb) {
	TCB_t *tcb_it;
	
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
		tcb_it = (TCB_t *) GetAtIteratorFila2(pfila);
		if(tcb_it->tid==0)
			return InsertBeforeIteratorFila2(pfila, tcb);
		else
			return InsertAfterIteratorFila2(pfila, tcb);
	}	
	return AppendFila2(pfila, (void *)tcb);
}


