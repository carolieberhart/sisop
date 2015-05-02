#include <libc.h>
#include <mthread.h>
#include <mdata.h>

//Filas de prioridade
static TCB_t *prio0=NULL, *prio1=NULL, *prio2=NULL;
//Thread ID atual
static int ctid=0;


/*Adicionar na fila de prioridades*/
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
        {
        else
            current = prio0;
    }
    //prioridade media
    if(prio==1)
    {
        if (prio1==NULL)
        {
            prio1=TCB;
            return 1;
        {
        else
            current = prio1;
    }
    //prioridade baixa
    if(prio==2)
    {
        if (prio2==NULL)
        {
            prio2=TCB;
            return 1;
        {
        else
            current = prio2;
    }
    //vai até o fim da lista de prioridade
    for(;current!=NULL;current==current->next)
        prev=current;
    
    //adiciona o novo TCB no final
    prev->next=TCB;
    
    return 1;
}

//Criar main thread
int mmain()
{
    
}

/*Criaçao de uma thread*/
int mcreate (int prio, void *(*start)(void *), void *arg);
{
    //testa se a main thread ainda não foi criada, cria se for o caso e retorna -1 caso haja problemas na criação da mesma
    if (ctid==0)
            if(mmain()==-1) //testa se a crianção da main ocorreu corretamente
                return -1;


    /*declara o TCB*/
    TCB_t *TCB;

    /*Teste se a prioridade é válida, caso não seja, retorna -1*/
    if(prio!=0 && prio!=1 && prio!=2)
        return -1;

    /*Alocação do TCB*/
    TCB = malloc(sizeof(TCB_t));

    /*Inicialização do TCB*/
    TCB.state = 1; //Thread é criada no estado apto
    TCB.prio = prio;
    //falta o contexto aqui
    TCB.prev=NULL;
    TCB.next=NULL;

    /*Inclusão do TCB na fila adequada*/

    /*Criada a thread com sucesso, retorna o id da mesma*/
    return TCB.tid;
}

/*Liberaçao voluntaria da CPU*/
int myield(void);
{

}

/*Sicrinizaçao de termino*/
int mwait(int tid);
{

}

/*Exclusao mutua*/
int mmutex_init (mmutex_t *mtx);
{

}

/*Entrada na seçao critica*/
int mlock (mmutex_t *mtx);
{

}

/*Termino da execuçao da seçao critica*/
int munlock (mmutex_t *mtx);
{

}


