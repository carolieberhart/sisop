#include <stdio.h>
#include <stdlib.h>
#include "../include/mthread.h"

/*

Este teste demonstra como é possível criar um número "infinito" de threads, até que a memória fique cheia.
Ao chamar o programa é passado um argumento arg do tipo inteiro que cria um número 'arg' de threads. Se nada
for informado, o número default de threads criadas é 20.

*/
void doSomething(void *arg)
{

    printf("doing something  %i...\n", *(int*)arg);
    *(int*)arg+=1;

}


int main(int arg, char **argcv){

int nT=20;
	if(arg > 0)
		nT=arg;

int *threads = (int*)malloc(nT*sizeof(int));
int i;
int n=0;


if(threads==NULL)
    return ;

for(i=0;i<nT;i++){
    threads[i]=mcreate(1, doSomething, &n);
}





printf("\nEnd of main...\n");


}
