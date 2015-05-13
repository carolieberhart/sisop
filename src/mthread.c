#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include "../include/mthread.h"
#include "../include/mdata.h"

#define PRIOR0 0
#define PRIOR1 1
#define PRIOR2 2
#define BLOCK 3

//VARIÁVEIS

//Filas de prioridade: 0=alta, 1=média, 2=baixa (juntas compões a fila de aptos)
TCB_t* prio0 = 0;
TCB_t* prio1 = 0;
TCB_t* prio2 = 0;

//Thread ID atual (incrementa cada vez q cria uma thread)
int ctid;

//TCB da thread em execução
TCB_t* executing_thread=0;

//Fila de threads bloqueadas
TCB_t* blocked_threads=0;

//Fila de ids de threads q já possuem uma thread esperando por elas
WAIT_t* being_waited=0;

//Verificador de inicialização da biblioteca
static int lib_init=FALSE;

//Contexto de término das threads
ucontext_t* retfunction=0;

//Contexto de retorno das threads
ucontext_t* retcontext=0;

//Contexto do escalonador
ucontext_t* scheduler_context=0;




//FUNÇÕES DE VERIFICAÇÃO DE THREADS SENDO ESPERADAS

//Verifica se já existe uma thread que espera por esse tid. Retorno: 0=encontrou, -1=não encontrou
int WAIT_contains(int tid)
{
    WAIT_t* current;
    current = being_waited;

    if(current==0) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current!=0)
        {
            if(current->waited==tid)
                return 0;
            current = current->next;
        }
    }
    return -1;
}

//Procura pela thread que está bloqueada por uma thread especifica (tio) e move-a para a fila de aptos. Retorno: 0=thread existia e foi movida e -1=thyread não existia ou thread não foi movida.
int WAIT_remove(int tid)
{
    WAIT_t* current= 0;
	WAIT_t* prev = 0;
    current = being_waited;
	printf("aaaaaaaaaaa%d", tid);
    if(current==0) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current != 0)
        {
            if(current->waited == tid)
            {
				printf("AAAAAAAAAAA");
				if(prev == 0)
					being_waited = 0;
				else
                	prev->next = current->next;
                //TCB_remove(BLOCK, tid);
                add_prioQ(current->waiting);
				current = 0;
            }
			else
			{
		        prev = current;
		        current = current->next;
			}
        }
    }

    return 0;
}

//Adiciona na lista de threads sendo esperadas. Retorno 0=sucesso -1=erro
int WAIT_add(WAIT_t* WAIT)
{
    //Adiciona na fila de bloqueados
    if(TCB_enqueue(BLOCK, WAIT->waiting) != 0)
        return -1;

    //Verifica se a fila está vazia.
    if(being_waited == 0)
    {
        being_waited=WAIT;
        return 0;
    }
    else //caso não esteja
    {
        WAIT->next = being_waited;
        being_waited = WAIT;
        return 0;
    }
}

//FUNÇÕES DE MANEJAMENTO DAS FILAS DE APTOS E DE BLOQUEADOS

TCB_t* TCB_getlist(int prior)
{

    switch(prior)
    {
    case PRIOR0:
        return prio0;
        break;

    case PRIOR1:
        return prio1;
        break;

    case PRIOR2:
        return prio2;
        break;    
	
	case BLOCK:
        return blocked_threads;
        break;

    default:
        return 0;
        break;

    }
}

//Verifica se um elemento existe em uma fila encadeada utilizando seu tid. Retorno: 0=encontrou, -1=não encontrou
int TCB_contains(int prior, int tid)
{

	TCB_t* aqueue = TCB_getlist(prior);
    TCB_t* current;

    current = aqueue;

    if(current==0) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current != 0)
        {
            if(current->tid == tid)
                return 0;
            current = current->next;
        }
    }
    return -1;
}

//Retira e retorna o TCB do primeiro elemento da FIFO. Retorno: TCB=se a fila não estava vazia, NULL=fila estava vazia.
TCB_t* TCB_dequeue(int prior)
{
	TCB_t* aqueue = TCB_getlist(prior);
    if(aqueue != 0)
	{
    	switch(prior)
				{
				case PRIOR0:
					prio0 = aqueue->next;
					break;

				case PRIOR1:
					prio1 = aqueue->next;
					break;

				case PRIOR2:
					prio2 = aqueue->next;
					break;    
	
				case BLOCK:
					blocked_threads = aqueue->next;
					break;

				default:
					return 0;
					break;
				}
	}

    return aqueue;
}

//Adiciona um TCB no fim da fila. Retorno: 0=sucesso, -1=erro (possível erro: o TCB já existe na fila)
int TCB_enqueue(int prior, TCB_t* TCB)
{
	TCB_t* aqueue = TCB_getlist(prior);
    if(aqueue == 0) //Fila vazia
    {
            switch(prior)
				{
				case PRIOR0:
					prio0 = TCB;
					break;

				case PRIOR1:
					prio1 = TCB;
					break;

				case PRIOR2:
					prio2 = TCB;
					break;    
	
				case BLOCK:
					blocked_threads = TCB;
					break;

				default:
					return 0;
					break;
				}
        return 0;
    }
    else //Fila não vazia
    {
    	TCB_t* current;

        current=aqueue;
        while(current->next != 0) //Verifica se algum TCB com aquele tid já existe na fila
        {
            if(current->tid == TCB->tid)
                return -1;
            current = current->next;
        }
        current->next = TCB;
        return 0;
    }

}

//Remove um TCB da lista (ele pode estar em qualquer posição). Retorno: TCB=TCB removido da lista, NULL=caso o TCB não esteja na fila ou ela esteja vazia
TCB_t* TCB_remove(int prior, int tid)
{
	TCB_t* aqueue = TCB_getlist(prior);
    TCB_t* current;
	TCB_t* prev;

    current = aqueue;

    if(current==0)//Fila vazia
        return 0;
    else //Fila não vazia
    {
        while(current != 0)
        {
            if(current->tid==tid)
            {
                prev->next = current->next;
                return current;
            }
            prev = current;
            current = current->next;
        }
    }
    return 0;
}

//Função que adiciona um TCB na sua fila de prioridades. Retorno: 0=sucesso, -1=erro
int add_prioQ (TCB_t* TCB)
{
    //prioridade alta
    if(TCB->prio==0)
	{
        if(TCB_enqueue(PRIOR0, TCB)==0)
            return 0;
	}
    //prioridade media
    else if(TCB->prio==1)
	{
        if(TCB_enqueue(PRIOR1, TCB)==0)
            return 0;
	}
    //prioridade baixa
    else if(TCB->prio==2)
	{
        if(TCB_enqueue(PRIOR2, TCB)==0)
            return 0;
	}
    else
        return -1;

    return -1;
}

//Procura um thread ID nas filas de proridades. Retorno 0=achou, -1=não achou
int search_prioQs(int tid)
{
    if(TCB_contains(PRIOR0, tid) == 0 || TCB_contains(PRIOR1, tid) == 0 || TCB_contains(PRIOR2, tid) == 0)
        return 0;
    else
        return -1;
}

//Pega o primeiro TCB da fila de maior prioridade. Retorno: TCB:sucesso, NULL=todas as filas de prioridades estão vazias.
TCB_t* mfifo()
{
    if(prio0 != 0)
        return TCB_dequeue(PRIOR0);
    else if(prio1 != 0)
        return TCB_dequeue(PRIOR1);
    else if(prio2 != 0)
        return TCB_dequeue(PRIOR2);
    else
        return 0;
}


//FUNÇÕES BÁSICAS PEDIDAS NA ESPECIFICAÇÃO DO TRABALHO

//Criar main thread
int mmain()
{
	//prio0 = calloc(1, sizeof(TCB_t));
	//prio1 = calloc(1, sizeof(TCB_t));
	//prio2 = calloc(1, sizeof(TCB_t));
	//executing_thread = calloc(1, sizeof(TCB_t));
	//blocked_threads = calloc(1, sizeof(TCB_t));
	//being_waited = calloc(1, sizeof(WAIT_t));
	retcontext = calloc(1, sizeof(ucontext_t));
	//scheduler_context = calloc(1, sizeof(ucontext_t));

	lib_init=TRUE;

    //Criar o TCB da main
    TCB_t* TCB = calloc(1, sizeof(TCB_t));

    //inicialização do TCB da thread main
    TCB->tid=0;
    TCB->state=0;
    TCB->prio=0;

    if(getcontext(&(TCB->context))==-1) //se ocorrer erro na obtenção do contexto
    {
        free(TCB);
        return -1;
    }

    TCB->next=0;
    TCB->prev=0;

    //atualiza os IDs disponíveis, para sinalizar que a main thread já foi criada.
    ctid++;

    //atualiza o estado para executando
    TCB->state=2;

	executing_thread = TCB;
    return 0; //criado com sucesso
}

/*Criaçao de uma thread*/
int mcreate(int prio, void *(*start)(void *), void *arg)
{
    //Testa se a biblioteca já foi inicializada (primeira chamada)
    if (lib_init==FALSE)
        if(mmain() == -1) //Testa se a inicialização da biblioteca ocorreu corretamente
            return -1;

    /*Teste se a prioridade é válida, caso não seja, retorna -1*/
    if(prio != 0 && prio != 1 && prio != 2)
        return -1;

    /*Alocação do TCB*/
    TCB_t* TCB = calloc(1, sizeof(TCB_t));
	retfunction = calloc(1, sizeof(ucontext_t));
	retcontext = calloc(1, sizeof(ucontext_t));

	

    //Salva o contexto para linkar no retorno.
	getcontext(retfunction);
	retfunction->uc_stack.ss_sp = calloc(SIGSTKSZ, sizeof(char));
	retfunction->uc_stack.ss_size = (SIGSTKSZ * sizeof(char));
	makecontext(retfunction, &set_ended, 0);


	//Cria o contexto da thread com e linka o retorno.
	getcontext(retcontext);
	retcontext->uc_link = retfunction;
	retcontext->uc_stack.ss_sp = calloc(SIGSTKSZ, sizeof(char));
	retcontext->uc_stack.ss_size = (SIGSTKSZ * sizeof(char));
	//make context
	makecontext(retcontext, (void(*)(void))start, 1, arg);

	TCB->context = *retcontext;
    /*Inicialização do TCB*/
    TCB->tid = ctid;
    TCB->state = 1; //Thread é criada no estado apto
    TCB->prio = prio;
    TCB->prev = 0;
    TCB->next = 0;


    //atualiza o ctid
    ctid++;

    /*Inclusão do TCB na fila adequada*/
    add_prioQ(TCB);

    /*Criada a thread com sucesso, retorna o id da mesma*/
    return TCB->tid;
}

/*Liberaçao voluntaria da CPU*/
int myield(void)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (lib_init==FALSE)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;

    //muda o estado da thread para apto
    executing_thread->state=1;

    mdispatcher();
	return -1;
}

/*Sicrinizaçao de termino*/
int mwait(int tid)
{
    //checa se o tid não é do próprio processo
    if(tid == executing_thread->tid)
        return -1;

    //verifica se o número do tid é válido
    if(tid >= 0 && tid <= ctid)
    {
        //verifica se o já não está sendo esperado por outro processo
        if(WAIT_contains(tid)!=0)
        {
            //verifica se o processo existe na fila de aptos
            if(search_prioQs(tid) || TCB_contains(BLOCK, tid))
            {
                //inicializa o novo item da fila
                WAIT_t* wthread = calloc(1,sizeof(WAIT_t));
                wthread->next=0;
                wthread->waited=tid;
                wthread->waiting=executing_thread;

                //adiciona o thrad id à lista de threads sendo esperadas
                if(WAIT_add(wthread) != 0)
                    return -1;

                //libera a thread em execução
                executing_thread->state = 3; //bloqueado

                mdispatcher();

                return 0;
            }
            else //thread a ser esperada não foi encontrada em nenyma fila
                return -1;
        }
        else //thread já está sendo esperada por outra thread
            return -1;
    }
    else //tid inválido
        return -1;

}

/*Exclusao mutua*/
int mmutex_init (mmutex_t *mtx)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;
	return -1;
}

/*Entrada na seçao critica*/
int mlock (mmutex_t *mtx)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;
	return -1;
}

/*Termino da execuçao da seçao critica*/
int munlock (mmutex_t *mtx)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;
	return -1;
}

//função que controla a execução das threads de acordo com as filas e estados, nao sei se é necessaria :s
void mdispatcher ()
{
        TCB_t* TCB;


        if(executing_thread->state==4)//Estado de término
        {
            WAIT_remove(executing_thread->tid);
            //free(executing_thread);
            TCB=mfifo();
			executing_thread = TCB;
			printf("\n%d\n", TCB->tid);
            if(TCB!=0)
                setcontext(&(TCB->context));
        }
        else if (executing_thread->state==3)//Estado bloqueado
        {
			retcontext = &(executing_thread->context);
            TCB_enqueue(BLOCK, executing_thread);
            TCB = mfifo();
			executing_thread = TCB;
			printf("\n%d\n", TCB->tid);
            if(TCB!=0)
                swapcontext(retcontext, &(TCB->context));			
        }
        else if(executing_thread->state==1)//Estado apto
        {
			retcontext = &(executing_thread->context);
            add_prioQ(executing_thread);
            TCB = mfifo();
			executing_thread = TCB;
			printf("\n%d\n", TCB->tid);
            if(TCB!=0)
                swapcontext(retcontext, &(TCB->context));
        }
}

//Set o contexto do TCB como 4=Término
void set_ended ()
{
    executing_thread->state = 4;
    mdispatcher();
}