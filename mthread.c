#include <stdlib.h>
#include <ucontext.h>

//OBSERVAÇÃO
//eu coloquei a verificação da existencia da main e a chamada da criação dela em todas as funções, mas nao sei se isso seria correto :)

//Estruturas

typedef struct TCB { 
	int  tid;		// identificador da thread 
	int  state;	// estado em que a thread se encontra 
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término 
	int  prio;		// prioridade da thread (0:alta; 1: média, 2:baixa) 
	ucontext_t   context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
	struct TCB   *prev;		// ponteiro para o TCB anterior da lista 
	struct TCB   *next;		// ponteiro para o próximo TCB da lista 
} TCB_t; 

typedef struct mutex {
	int	flag; 					// indica se o mutex foi adquirido ou não (0: livre; 1: ocupado)
	struct TCB	*first, *last; 	// ponteiros para lista de threads bloqueadas no mutex
} mmutex_t;

typedef struct WAIT
{
    int waited; //id da thread sendo esperada
	struct TCB *waiting; //TCB da thread que está esperando
    struct WAIT *next;
}WAIT_t;

//VARIAVEIS

//Filas de prioridade
TCB_t *prio0=NULL, *prio1=NULL, *prio2=NULL;

//Thread ID atual (incrementa cada vez q cria uma thread)
int ctid=0;

//TCB da thread em execução
TCB_t *executing_thread=NULL;

//Fila de threads bloqueadas
TCB_t *blocked_threads=NULL;

//Fila de ids de threads q já possuem uma thread esperando por elas
WAIT_t *being_waited=NULL;


//Funções

//Verifica se já existe uma thread que espera por esse tid (0 se encontrou, -1 se não encontrou)
int WAIT_contains(int tid)
{
	WAIT_t *current;
	current = being_waited;
	
    //Procura na queue
	while(current != NULL)
	{
		if(current->waited == tid)
			return 0;
		current = current->next;
	}
	
	return -1;
}

//Procura por threads que estão bloqueadas por esta tid e move para a fila de aptos. retorna 0 se existia e -1 se não.
int WAIT_remove(int tid)
{
	WAIT_t *current, *prev;
	current = being_waited;
	
    //Procura na queue
	while(current != NULL)
	{
		if(current->waited == tid)
		{
			prev->next = current->next;
			TCB_remove(blocked_threads, tid);
			TCB_enqueue(current->waiting);
			return 0;
		}
		prev = current;
		current = current->next;
	}
	
	return -1;
}

//Adiciona na lista.
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
		being_waited->next = WAIT;
        return 0;
    }
}

//Verifica se um elemento existe em uma fila encadeada pelo seu tid (0 se encontrou, -1 se não encontrou)
int TCB_contains(TCB_t *queue, int tid)
{
	TCB_t *current;
	
	current = queue;
	
    //Procura na queue
	while(current != NULL)
	{
		if(current->tid == tid)
			return 0;
		current = current->next;
	}
	
	return -1;
}

//Retira e retorna o tid do primeiro elemento da FIFO, retorna NULL caso a fila esteja vazia.
TCB_t* TCB_dequeue(TCB_t *queue)
{
    TCB_t *first;
	first = queue;
	
	if(queue != NULL)
		queue = queue->next;
	
	return first;
}

//Adiciona um TCB no fim da queue. retona -1 caso já esteja na fila.
int TCB_enqueue(TCB_t *queue, TCB_t *TCB)
{
	TCB_t *current;
	
	//Verifica se a fila está vazia.
    if(queue == NULL)
    {
        queue=TCB;
        return 0;
    }
    else //caso não esteja
    {
        current=queue;

		//Procura o último elemento da fila.
        while(current->next != NULL)
		{
			//Já existe na fila.
			if(current->tid == TCB->tid)
				return -1;
			current = current->next;
		}

		current->next = TCB;

        return 0;
    }
}

//Remove um TCB da lista. retorna NULL caso a lista esteja vazia.
TCB_t* TCB_remove(TCB_t *queue, int tid)
{
	TCB_t *current, *prev;
	
	current = queue;
	
    //Procura na queue
	while(current != NULL)
	{
		if(current->tid == tid)
		{
			prev->next = current->next
			return current;
		}
		prev = current;
		current = current->next;
	}
	
	return current;
}

//Adicionar na fila de prioridades [TESTADO]
int add_prioQ (TCB_t *TCB)
{
    //prioridade alta
    if(TCB->prio==0)
		TCB_enqueue(prio0, TCB);
    //prioridade media
    else if(TCB->prio==1)
		TCB_enqueue(prio1, TCB);
    //prioridade baixa
    else if(TCB->prio==2)
		TCB_enqueue(prio2, TCB);
    else
        return -1;
	
	return 0;
}

//procura um thread ID nas filas de proridades. retorno 0=achou, -1=não achou
int search_prioQs(int tid)
{
	if(TCB_contains(prio0, tid) == 0 || TCB_contains(prio1, tid) == 0 || TCB_contains(prio2, tid) == 0)
		return 0;
	else
		return -1;
}

//Pegar o primeiro TCB da fila de maior prioridade [TESTADO], retorna NULL caso estejam vazias.
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
	
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid == 0)
		if(mmain() == -1) //testa se a crianção da main ocorreu corretamente
			return -1;

    /*Alocação do TCB*/
    TCB_t* TCB = (TCB_t*)  malloc(sizeof(TCB_t));

    /*Inicialização do TCB*/
    TCB->tid = ctid;
    TCB->state = 1; //Thread é criada no estado apto
    TCB->prio = prio;
    if(getcontext(&TCB->context) == -1)//se ocorrer falha na hora de recuperar o contexto
    {
        free(TCB);
        return -1;
    }
    TCB->prev = NULL;
    TCB->next = NULL;

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