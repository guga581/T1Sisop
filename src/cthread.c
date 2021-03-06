#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucontext.h>

#include "../include/support.h"
#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/insert.h"

#define stackSize SIGSTKSZ
#define defaultTID -1


//variaveis universais

int firstExec = 1;
int nroTID = 1;
int retThread = 0;
int startExec = 0;
int stopExec = 0;

FILA2 aptos;
FILA2 bloqueados;
FILA2 aptossusp;
FILA2 bloqueadossusp;
FILA2 cjoinQueue;

TCB_t *exec;
TCB_t *threadMain;

ucontext_t endThread, dispatch_ctx;

TCB_t *setNewExec(){

	if (FirstFila2(&aptos) == 0){

		TCB_t *unlockedThread = NULL;
		if(GetAtIteratorFila2(&aptos) != NULL){
			unlockedThread = (TCB_t *) GetAtIteratorFila2(&aptos);
			exec = unlockedThread;			
		}
		else
			exec = NULL;

	}

	else
		return NULL;

	return exec;
}

//inicia proxima thread da fila de aptos

void dispatch(){	
	
	exec = setNewExec();
	if(exec != NULL){
	
		if(FirstFila2(&aptos) == 0)
			if(DeleteAtIteratorFila2(&aptos) == 0)
				setcontext(&exec->context);
	}
	else
		printf("Nao tem mais threads para serem executadas\n");		
}


//funcao que checa se nao ha mais de uma thread aguardando pelo mesmo tid
//caso positivo retorna 0 (Erro); caso negativo, 1 (Sucesso)

int checkJoin(int tid){

	if(FirstFila2(&bloqueados) == 0){
		TCB_t *threadInQueue = NULL;
		while(GetAtIteratorFila2(&bloqueados) != NULL){
		
			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueados);
			
			if(threadInQueue->tidCjoin == tid)
				return 1;
				
			if(NextFila2(&bloqueadossusp) != 0)
				break;			
		}
	}
	if(FirstFila2(&bloqueadossusp) == 0){
		TCB_t *threadInQueue = NULL;
		while(GetAtIteratorFila2(&bloqueadossusp) != NULL){
		
			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueadossusp);
			if(threadInQueue->tidCjoin == tid)
				return 1;
	
			else{
				if(NextFila2(&bloqueadossusp) != 0)
				{
					return 0;
				}
			}	
		}
		
		return 0;
	}
	else
		return 0;
}

void changeState(PFILA2 queue, TCB_t *threadGoingToQueue){

	//insere na fila indicada por queue

	if (exec != NULL)
		if(Insert(queue, threadGoingToQueue) != 0)
			printf("Nao conseguiu inserir em fila na changeState()\n");
			
	

}

//funcao que verifica se o tid da thread cujo termino esta sendo aguardado é válido
//veriricando na fila de aptos e bloqueados retorna 0 caso deu erro e 1 caso sucesso

int verifyCjoin(int tid, FILA2 queue){

	TCB_t *threadInQueue = NULL;

	while(GetAtIteratorFila2(&queue) != NULL){
		
		threadInQueue = (TCB_t *) GetAtIteratorFila2(&queue);
		if(threadInQueue->tid == tid){
			exec->tidCjoin = tid;	//indica o tid pela qual a thread que esta executando devera esperar
			exec->state = PROCST_BLOQ;
			
			//**THREAD QUE EXECUTAVA VAI PARA BLOQUEADO 
						
			return 1;
		}

		else
			if(NextFila2(&queue) != 0)
				return 0;
	}
	
	return 0;

}


void lookForTidinBlockedQueue(){
	if (FirstFila2(&bloqueados) == 0){
		
		TCB_t *threadInQueue = NULL;

		while(GetAtIteratorFila2(&bloqueados) != NULL){
			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueados);
			
			if (threadInQueue->tidCjoin == exec->tid){

				threadInQueue->state = PROCST_APTO;
				threadInQueue->tidCjoin = defaultTID;
				changeState(&aptos, threadInQueue);			//tira thread de bloqueado e coloca em aptos

				DeleteAtIteratorFila2(&bloqueados);			//exclui thread da fila de bloqueados

				return;				
				
			}
			if(NextFila2(&bloqueados) != 0)
				break;	
			
		}
	}
	
	if (FirstFila2(&bloqueadossusp) == 0){
		
		TCB_t *threadInQueue = NULL;
		while(GetAtIteratorFila2(&bloqueadossusp) != NULL){
			threadInQueue = (TCB_t *) GetAtIteratorFila2(&bloqueadossusp);
			
			if (threadInQueue->tidCjoin == exec->tid){

				threadInQueue->state = PROCST_APTO_SUS;
				threadInQueue->tidCjoin = defaultTID;
				changeState(&aptossusp, threadInQueue);			//tira thread de bloqueado-suspenso e coloca em aptos-suspenso

				DeleteAtIteratorFila2(&bloqueadossusp);			//exclui thread da fila de bloqueados-suspenso

				return;				
				
			}
			if(NextFila2(&bloqueadossusp) != 0)
				return;	
			
		}
	}

}


void fimThread(){


	printf("thread encerrando = %d\n", exec->tid);
		
	lookForTidinBlockedQueue(); //procura na fila de bloqueados alguma thread que esteja esperando pela que acabou

	//desaloca estrutura da thread para liberar memoria
	free(exec);
	exec = NULL;

	dispatch();
	
}

void initMain(){

	threadMain = (TCB_t*) malloc(sizeof(TCB_t));
		
	threadMain->tid = 0;	
	threadMain->prio = 0;	
	threadMain->state = PROCST_EXEC;	
	threadMain->tidCjoin = defaultTID;
	
	getcontext(&threadMain->context);
	exec = threadMain;

}

void initDispatch(){
	
	getcontext(&dispatch_ctx);

	dispatch_ctx.uc_link = 0;
	dispatch_ctx.uc_stack.ss_sp = (char*) malloc(stackSize);
	dispatch_ctx.uc_stack.ss_size = stackSize;

	makecontext(&dispatch_ctx, (void(*)(void))dispatch, 0);
}


void initFimThread(){

	getcontext(&endThread);

	endThread.uc_link = 0;
	endThread.uc_stack.ss_sp = (char*) malloc(stackSize);
	endThread.uc_stack.ss_size = stackSize;

	makecontext(&endThread, (void(*)(void))fimThread, 0);
}


int ccreate (void *(*start) (void*), void *arg, int zero){
	
	//primeira execucao cria contexto da main, do dispatcher;
	//inicializa filas e funcao que testa fim de thread 
	
	if(firstExec){
		
		initMain();
		initDispatch();
		initFimThread();
		

		//criando fila de aptos e bloqueados 

		if(CreateFila2(&aptos) != 0){
			printf("Nao criou a fila de aptos\n");
			return -1;
		}
		
		if(CreateFila2(&bloqueados) != 0){
			printf("Nao criou a fila de bloqueados\n");
			return -1;
		}	
		
		if(CreateFila2(&aptossusp) != 0){
			printf("Nao criou a fila de aptos-suspensos\n");
			return -1;
		}
		
		if(CreateFila2(&bloqueadossusp) != 0){
			printf("Nao criou a fila de bloqueados-suspensos\n");
			return -1;
		}
		
		firstExec = 0;	//variavel que indica que nao eh mais a primeira execucao
	}

	
	//criando nova thread

	TCB_t *novaThread;
	novaThread = (TCB_t*) malloc(sizeof(TCB_t));

	novaThread->prio = zero;
	novaThread->tid = nroTID;
	novaThread->state = PROCST_APTO;
	novaThread->tidCjoin = defaultTID;
		
	getcontext(&(novaThread->context));

	novaThread->context.uc_link = &endThread;
	novaThread->context.uc_stack.ss_sp = (char*) malloc(stackSize);
	novaThread->context.uc_stack.ss_size = stackSize;
	
	makecontext(&(novaThread->context), (void (*) (void))start, 1, arg);

	
	//Inserindo na Fila de Aptos;

	if (Insert(&aptos, novaThread) == 0)
		printf("Nova Thread inserida na fila de Aptos\n");
	else{
		printf("Erro na insercao na fila de Aptos\n");
		return -1;
	}

	nroTID++;
	
	return novaThread->tid;

}

int cjoin(int tid){
	
	int flagVerify;	

	//verifica fila de aptos e bloqueados para ver se o tid eh valido
	
	if ((FirstFila2(&aptos) == 0) || (FirstFila2(&bloqueados) == 0) || (FirstFila2(&aptossusp) == 0) || (FirstFila2(&bloqueadossusp) == 0) ){		//fila de bloqueados pode estar vazia, retornando 
											//algo diferente de 0

		flagVerify =  ( (verifyCjoin(tid, aptos)) || (verifyCjoin(tid, bloqueados)) || (verifyCjoin(tid, aptossusp)) || (verifyCjoin(tid, bloqueadossusp)));

		if (flagVerify == 0){				//se as duas hipoteses do condicional "ou" forem falsas, o retorno de 
			flagVerify = -1;			//cjoin deve ser -1
			return flagVerify;
		}
		flagVerify = flagVerify && checkJoin(tid);
		
		
		if(flagVerify == 0){
			exec->state= PROCST_BLOQ;
			changeState(&bloqueados, exec);
			swapcontext(&exec->context, &dispatch_ctx);

			return flagVerify;
		}
		else
			return -1;
	}
	
	else
		return -1;		
}

//altera estado da thread que ocupa atualmente a CPU (exec) para APTO;
//insere a thread na fila de aptos
//chama o dispatcher para executar a próxima thread da fila de aptos, salvando o contexto da main(swapcontext)
//variavel que indica o retorno para a thread main após o cyield (0 -> SUCESSO; -1 -> ERRO)

int cyield(){
	
	exec->state = PROCST_APTO;

	changeState(&aptos, exec);
	swapcontext(&exec->context, &dispatch_ctx);
	
	retThread = 1;

	if(retThread){
		retThread = 0;
		return 0;
	}
	else{
		retThread = 0;
		return -1;
	}
	
}

int csem_init(csem_t *sem, int count){

	sem->count = count;
	sem->fila  = (FILA2 *)malloc(sizeof(FILA2));

	if(CreateFila2(sem->fila) != 0){
		printf("Nao criou fila em csem_init!\n");
		return -1;
	}

	return 0;
}

int cwait(csem_t *sem){
	
	sem->count--;

	if(sem->count < 0){

		exec->state = PROCST_BLOQ;

		if(AppendFila2(sem->fila, (void *) exec) != 0){
			printf("Nao colocou no fim de sem->fila em cwait\n");
			return -1;
		}
		swapcontext(&exec->context, &dispatch_ctx);	
	} 
	
	return 0;
	
}


int csignal(csem_t *sem){

	sem->count++;

	if(FirstFila2(sem->fila) == 0){
		
		TCB_t *t_des = (TCB_t *) GetAtIteratorFila2(sem->fila);
		t_des->state = PROCST_APTO;
		
		changeState(&aptos, t_des);
		
		if(DeleteAtIteratorFila2(sem->fila) != 0){
			printf("Nao deletou da fila do semaforo em csignal\n");
			return -1;
		}

	}
		
	return 0;		  
}

//Funcao de identificacao
//retorna 0 se SUCESSO, -1 se ERRO
/*OBS*** DEIXAR ESSA FUNCAO COMO ULTIMA DO ARQUIVO cthread.c!!!
*/

/******************************************************************************
Parâmetros:
	tid:	identificador da thread a ser suspensa.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int csuspend(int tid){
	if (FirstFila2(&bloqueados) == 0){
		
		//compara tid a ser suspenso na lista de bloqueados
		do{
		TCB_t *ToSuspendThread = NULL;
			
			ToSuspendThread = (TCB_t *) GetAtIteratorFila2(&bloqueados);
			if(ToSuspendThread->tid==tid)
			{
				
				ToSuspendThread->state=PROCST_BLOQ_SUS;	//suspende a thread e coloca na lista de bloqueados-suspensos
				changeState(&bloqueadossusp, ToSuspendThread);
				DeleteAtIteratorFila2(&bloqueados);
				return 0;
			}
		
	} while(NextFila2(&bloqueados)== 0);
	}
	//compara tid a ser suspenso na lista de aptos

	if (FirstFila2(&aptos) == 0){
		do{
		TCB_t *ToSuspendThread = NULL;
		
			ToSuspendThread = (TCB_t *) GetAtIteratorFila2(&aptos);
			if(ToSuspendThread->tid==tid)
			{
				
				ToSuspendThread->state=PROCST_APTO_SUS; //suspende a thread e coloca na lista de aptos-suspensos
				changeState(&aptossusp, ToSuspendThread);
				DeleteAtIteratorFila2(&aptos);
				return 0;
			}
		
	} while(NextFila2(&aptos)== 0);
	
	}
		return -1;

}

/******************************************************************************
Parâmetros:
	tid:	identificador da thread que terá sua execução retomada.
Retorno:
	Se correto => 0 (zero)
	Se erro	   => Valor negativo.
******************************************************************************/
int cresume(int tid){
	if (FirstFila2(&bloqueadossusp) == 0){
		// pesquisa pela tid a ser retomada e devolve a fila de bloqueados
		do{
		TCB_t *ToResumeThread = NULL;
		
			ToResumeThread = (TCB_t *) GetAtIteratorFila2(&bloqueadossusp);
			if(ToResumeThread->tid==tid)
			{
				
				ToResumeThread->state=PROCST_BLOQ;
				changeState(&bloqueados, ToResumeThread);
				DeleteAtIteratorFila2(&bloqueadossusp);
				return 0;
			}
		
	} while(NextFila2(&bloqueadossusp)== 0);
	}
	
	// pesquisa pela tid a ser retomada e devolve a fila de aptos
	
	if (FirstFila2(&aptossusp) == 0){
		
		do{
		TCB_t *ToResumeThread = NULL;
		
			ToResumeThread = (TCB_t *) GetAtIteratorFila2(&aptossusp);
			
			if(ToResumeThread->tid==tid)
			{
			
				ToResumeThread->state=PROCST_APTO;
				changeState(&aptos, ToResumeThread);
				DeleteAtIteratorFila2(&aptossusp);
				return 0;
			}
		
	} while(NextFila2(&aptossusp)== 0);
	
	}
		return -1;
}

int cidentify(char *name, int size){

	name = "Gustavo Francisco\t00273173\nMarcellus\t00281984\n";

	if(sizeof(name) < size)

		if(puts(name))
			return 0;
		else
			return -1;
	else
		return -1;

}

