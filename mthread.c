#include <mthread.h>
#include <mdata.h>
//OBSERVAÇÃO
//eu coloquei a verificação da existencia da main e a chamada da criação dela em todas as funções, mas nao sei se isso seria correto :)





//Filas de prioridade
static TCB_t *prio0=NULL, *prio1=NULL, *prio2=NULL;

//Thread ID atual (incrementa cada vez q cria uma thread)
static int ctid=0;

//TCB da thread em execução
static TCB_t *executing_thread=NULL;

//Adicionar na fila de prioridades [TESTADO]
int add_prioQ (int prio, TCB_t *TCB)
{
    TCB_t *prev, *current;
    prev=NULL;
    //prioridade alta
    if(prio==0)
    {
        if (prio0==NULL)
        {
            prio0=TCB;
            return 1;
        }
        else
            current = prio0;
    }
    //prioridade media
    else if(prio==1)
    {
        if (prio1==NULL)
        {
            prio1=TCB;
            return 1;
        }
        else
            current = prio1;
    }
    //prioridade baixa
    else if(prio==2)
    {
        if (prio2==NULL)
        {
            prio2=TCB;
            return 1;
        }
        else
            current = prio2;
    }
    else
        return -1;


    //vai até o fim da lista de prioridade
    for(;current!=NULL;current=current->next)
        prev=current;

    //adiciona o novo TCB no final
    prev->next=TCB;

    return 0;
}


//Pegar o primeiro TCB da fila de maior prioridade [TESTADO]
TCB_t* mfifo()
{
    TCB_t *temp;
    if(prio0!=NULL)
    {
        temp=prio0;
        prio0=prio0->next;
    }
    else if(prio1!=NULL)
    {
        temp=prio1;
        prio1=prio1->next;
    }
    else
    {
        temp=prio2;
        prio2 = prio2->next;
    }
    return temp;
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

     if(getcontext(TCB->context)==-1) //se ocorrer erro na obtenção do contexto
     {
         free(TCB);
         return -1;
     }

     TCB->next=NULL;
     TCB->prev=NULL;

     //atualiza os IDs disponíveis, para sinalizar que a main thread já foi criada.
     ctid++;

     //atualiza o estado para apto
     TCB->state=1;

     //adicionando o TCB da main na fila de prioridades
     if(add_prioQ(TCB, 0)==0)
        return 0; //criado com sucesso
     else
        return -1;
}

/*Criaçao de uma thread*/
int mcreate (int prio, void *(*start)(void *), void *arg);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;



    /*Teste se a prioridade é válida, caso não seja, retorna -1*/
    if(prio!=0 && prio!=1 && prio!=2)
        return -1;

    /*Alocação do TCB*/
    TCB_t* TCB = (TCB_t*)  malloc(sizeof(TCB_t));

    /*Inicialização do TCB*/
    TCB->tid=ctid;
    TCB->state = 1; //Thread é criada no estado apto
    TCB->prio = prio;
    if(getcontext(TCB->context)==-1)//se ocorrer falha na hora de recuperar o contexto
    {
        free(TCB);
        return -1;
    }
    TCB->prev=NULL;
    TCB->next=NULL;

    //atualiza o ctid
    ctid++;

    /*Inclusão do TCB na fila adequada*/
    add_prioQ(prio, TCB);
    /*Criada a thread com sucesso, retorna o id da mesma*/
    return TCB.tid;
}

/*Liberaçao voluntaria da CPU*/
int myield(void);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;

    //muda o estado da thread para apto
    executing_thread->state=1;
    //pega o context e adiciona na fila de prioridades correspondente, ao mesmo tempo que testa pra ver se isso ocorreu com sucesso
    if(getcontext(executing_thread->context)==0 && add_prioQ(executing_thread->prio, executing_thread)==0)
    {
        executing_thread=NULL;
        return 0;
    }
    else
        return -1;
}

/*Sicrinizaçao de termino*/
int mwait(int tid);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;
}

/*Exclusao mutua*/
int mmutex_init (mmutex_t *mtx);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;
}

/*Entrada na seçao critica*/
int mlock (mmutex_t *mtx);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;
}

/*Termino da execuçao da seçao critica*/
int munlock (mmutex_t *mtx);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;
}

//função que controla a execução das threads de acordo com as filas e estados
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
