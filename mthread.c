#include <stdlib.h>
#include <ucontext.h>
#include "mthread.h"
#include "mdata.h"

//VARIÁVEIS

//Filas de prioridade: 0=alta, 1=média, 2=baixa (juntas compões a fila de aptos)
TCB_t *prio0=NULL, *prio1=NULL, *prio2=NULL;

//Thread ID atual (incrementa cada vez q cria uma thread)
int ctid=0;

//TCB da thread em execução
TCB_t *executing_thread=NULL;

//Fila de threads bloqueadas
TCB_t *blocked_threads=NULL;

//Fila de ids de threads q já possuem uma thread esperando por elas
WAIT_t *being_waited=NULL;

//Verificador de inicialização da biblioteca
static bool lib_init=FALSE;

//Contexto de término das threads
ucontext_t *retfunction=NULL;

//Contexto do escalonador
ucontext_t *scheduler_context=NULL;




//FUNÇÕES DE VERIFICAÇÃO DE THREADS SENDO ESPERADAS

//Verifica se já existe uma thread que espera por esse tid. Retorno: 0=encontrou, -1=não encontrou
int WAIT_contains(int tid)
{
    WAIT_t *current;
    current = being_waited;

    if(current==NULL) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current!=NULL)
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
    WAIT_t *current, *prev;
    current = being_waited;

    if(current==NULL) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current != NULL)
        {
            if(current->waited == tid)
            {
                prev->next = current->next;
                TCB_remove(blocked_threads, tid);
                if((current->waiting)->prio==0)
                    if(TCB_enqueue(prio0,current->waiting)==0)
                        return 0;
                    else if((current->waiting)->prio==1)
                        if(TCB_enqueue(prio1,current->waiting)==0)
                            return 0;
                        else if((current->waiting)->prio==2)
                            if(TCB_enqueue(prio2,current->waiting)==0)
                                return 0;
            }
            prev = current;
            current = current->next;
        }
    }

    return -1;
}

//Adiciona na lista de threads sendo esperadas. Retorno 0=sucesso -1=erro
int WAIT_add(WAIT_t *WAIT)
{
    //Adiciona na fila de bloqueados
    if(TCB_enqueue(blocked_threads, WAIT->waiting) != 0)
        return -1;

    //Verifica se a fila está vazia.
    if(being_waited == NULL)
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

//Verifica se um elemento existe em uma fila encadeada utilizando seu tid. Retorno: 0=encontrou, -1=não encontrou
int TCB_contains(TCB_t *aqueue, int tid)
{
    TCB_t *current;

    current = aqueue;

    if(current==NULL) //Fila vazia
        return -1;
    else //Fila não vazia
    {
        while(current != NULL)
        {
            if(current->tid == tid)
                return 0;
            current = current->next;
        }
    }
    return -1;
}

//Retira e retorna o TCB do primeiro elemento da FIFO. Retorno: TCB=se a fila não estava vazia, NULL=fila estava vazia.
TCB_t* TCB_dequeue(TCB_t *aqueue)
{
    TCB_t *first;
    first = aqueue;

    if(aqueue != NULL)
        aqueue = aqueue->next;

    return first;
}

//Adiciona um TCB no fim da fila. Retorno: 0=sucesso, -1=erro (possível erro: o TCB já existe na fila)
int TCB_enqueue(TCB_t *aqueue, TCB_t *TCB)
{
    TCB_t *current;

    if(aqueue == NULL) //Fila vazia
    {
        aqueue=TCB;
        return 0;
    }
    else //Fila não vazia
    {
        current=aqueue;
        while(current->next != NULL) //Verifica se algum TCB com aquele tid já existe na fila
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
TCB_t* TCB_remove(TCB_t *aqueue, int tid)
{
    TCB_t *current, *prev;

    current = aqueue;

    if(current==NULL)//Fila vazia
        return NULL;
    else //Fila não vazia
    {
        while(current != NULL)
        {
            if(current->tid==tid)
            {
                prev->next = current->next
                return current;
            }
            prev = current;
            current = current->next;
        }
    }
    return NULL;
}

//Função que adiciona um TCB na sua fila de prioridades. Retorno: 0=sucesso, -1=erro
int add_prioQ (TCB_t *TCB)
{
    //prioridade alta
    if(TCB->prio==0)
        if(TCB_enqueue(prio0, TCB)==0)
            return 0;
    //prioridade media
    else if(TCB->prio==1)
        if(TCB_enqueue(prio1, TCB)==0)
            return 0;
    //prioridade baixa
    else if(TCB->prio==2)
        if(TCB_enqueue(prio2, TCB)==0)
            return 0;
    else
        return -1;

    return -1;
}

//Procura um thread ID nas filas de proridades. Retorno 0=achou, -1=não achou
int search_prioQs(int tid)
{
    if(TCB_contains(prio0, tid) == 0 || TCB_contains(prio1, tid) == 0 || TCB_contains(prio2, tid) == 0)
        return 0;
    else
        return -1;
}

//Pega o primeiro TCB da fila de maior prioridade. Retorno: TCB:sucesso, NULL=todas as filas de prioridades estão vazias.
TCB_t* mfifo()
{
    if(prio0 != NULL)
        return TCB_dequeue(prio0);
    else if(prio1 != NULL)
        return TCB_dequeue(prio1);
    else if(prio2 != NULL)
        return TCB_dequeue(prio2);
    else
        return NULL;
}


//FUNÇÕES BÁSICAS PEDIDAS NA ESPECIFICAÇÃO DO TRABALHO

//Criar main thread
int mmain()
{
    //Criar o TCB da main
    TCB_t* TCB = (TCB_t*)  malloc(sizeof(TCB_t));

    //inicialização do TCB da thread main
    TCB->tid=0;
    TCB->state=0;
    TCB->prio=0;

    if(getcontext(&TCB->context)==-1) //se ocorrer erro na obtenção do contexto
    {
        free(TCB);
        return -1;
    }

    TCB->next=NULL;
    TCB->prev=NULL;

    //atualiza os IDs disponíveis, para sinalizar que a main thread já foi criada.
    ctid++;

    //atualiza o estado para executando
    TCB->state=2;

    //adicionando o TCB da main na fila de prioridades
    if(add_prioQ(TCB)==0)
        return 0; //criado com sucesso
    else
        return -1;
}

/*Criaçao de uma thread*/
int mcreate(int prio, void *(*start)(void *), void *arg)
{
    /*Teste se a prioridade é válida, caso não seja, retorna -1*/
    if(prio != 0 && prio != 1 && prio != 2)
        return -1;

    //Testa se a biblioteca já foi inicializada (primeira chamada)
    if (lib_init==FALSE)
        if(mthread_init() == -1) //Testa se a inicialização da biblioteca ocorreu corretamente
            return -1;

    /*Alocação do TCB*/
    TCB_t* TCB = (TCB_t*)  malloc(sizeof(TCB_t));

    //Alocação do espaço de contexto do tcb e preenchimento com o contexto.
    TCB->context = (ucontext_t*) malloc(sizeof(ucontext_t));
    //Salva o contexto para linkar no retorno.
	getcontext(&retfunction);
	retfunction.uc_stack.ss_sp = malloc(SIGSTKSZ,sizeof(char));
	retfunction.uc_stack.ss_size = SIGSTKSZ * sizeof(char);
	makecontext(&retfunction, &set_ended, 0);

	//Cria o contexto da thread com e linka o retorno.
	getcontext(&retcontext);
	retcontext.uc_stack.ss_sp = malloc(SIGSTKSZ,sizeof(char));
	retcontext.uc_stack.ss_size = SIGSTKSZ * sizeof(char);
	retcontext.uc_link = retfunction;
	//make context
	makecontext(&retcontext, (void(*)(void))start, 1, arg);
    /*Inicialização do TCB*/
    TCB->tid = ctid;
    TCB->state = 1; //Thread é criada no estado apto
    TCB->prio = prio;
    TCB->prev = NULL;
    TCB->next = NULL;

    makecontext((TCB->context),  (void (*) (void)) start, 1, arg);

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
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;

    //muda o estado da thread para apto
    executing_thread->state=1;

    //pega o context e adiciona na fila de prioridades correspondente, ao mesmo tempo que testa pra ver se isso ocorreu com sucesso
    if(getcontext(&executing_thread->context)==0 && add_prioQ(executing_thread)==0)
    {
        //livra o executing thread do TCB que estava lá antes da chamada de myield
        executing_thread=NULL;

        //--------------
        //--DISPATCHER--
        //--------------

        return 0;
    }
    else
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
        if(WAIT_contains(tid)==0)
        {
            //verifica se o processo existe na fila de aptos
            if(search_prioQs(tid) || TCB_contains(blocked_threads, tid))
            {
                //atualiza o contexto da thread
                getcontext(&executing_thread->context);

                //inicializa o novo item da fila
                WAIT_t* wthread = (WAIT_t*)  malloc(sizeof(WAIT_t));
                wthread->next=NULL;
                wthread->waited=tid;
                wthread->waiting=executing_thread;

                //adiciona o thrad id à lista de threads sendo esperadas
                if(WAIT_add(wthread) != 0)
                    return -1;

                //libera a thread em execução
                executing_thread=NULL;

                //--------------
                //--DISPATCHER--
                //--------------

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
}

/*Entrada na seçao critica*/
int mlock (mmutex_t *mtx)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;
}

/*Termino da execuçao da seçao critica*/
int munlock (mmutex_t *mtx)
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
        if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
            return -1;
}

//FUNÇÕES DE CONTROLE DA BIBLIOTECA MTHREAD
int create_final_context ()
{

}


int mthread_init()
{


lib_init=TRUE;
}

//função que controla a execução das threads de acordo com as filas e estados, nao sei se é necessaria :s
int mdispatcher ()
{
    while(1)
    {
        //usa a função fifo para saber qual thread executar

        //executa a thread

        //recebe o resultado: block ou termino

        //se for termino consulta a lista de bloqueadas para ver se alguma thread esperava aquele termino, se sim, move para a fila de aptos


        //só vai retornar quando todas threads forem finalizadas e as filas estiverem todas vazias!!!
        if(prio0==NULL && prio1==NULL && prio2==NULL) //talvez falte testar pra ver se não tem nenhum processo bloqueado
            return 0;
    }


}

//Set o contexto do TCB como 4=Término
void set_ended ()
{
    executing_thread->state = 4;

    mdispatcher();
}
