
/*
 * mdata.h: arquivo de inclusão de uso apenas na geração da libmthread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida.
 *
 */
#ifndef __mdata__
#define __mdata__


#define STACK_SIZE 4096
#define TRUE 1
#define FALSE 0


/* NÃO ALTERAR ESSA struct */
typedef struct TCB {
	int  tid;		// identificador da thread
	int  state;	// estado em que a thread se encontra
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	int  prio;		// prioridade da thread (0:alta; 1: média, 2:baixa)
	ucontext_t   context;	// contexto de execução da thread (SP, PC, GPRs e recursos)
	struct TCB   *prev;		// ponteiro para o TCB anterior da lista
	struct TCB   *next;		// ponteiro para o próximo TCB da lista
} TCB_t;


//Estrutura que contrói a fila de threads que estão sendo esperadas.
typedef struct WAIT
{
    int waited; //id da thread sendo esperada
	struct TCB *waiting; //TCB da thread que está esperando
    struct WAIT *next;
}WAIT_t;

int WAIT_contains(int tid);
int WAIT_remove(int tid);
int WAIT_add(WAIT_t *WAIT);
int TCB_contains(TCB_t *aqueue, int tid);
TCB_t* TCB_dequeue(TCB_t *aqueue);
int TCB_enqueue(TCB_t *aqueue, TCB_t *TCB);
TCB_t* TCB_remove(TCB_t *aqueue, int tid);
int add_prioQ (TCB_t *TCB);
int search_prioQs(int tid);
TCB_t* mfifo();

#endif
