#include <stdio.h>
#include <stdlib.h>
#include "../include/mthread.h"


 void quem_sou_eu()
 {

     printf("Eu sou a thread  %i...\n", executing_thread->tid);

 }


 int main(int arg, char **argcv){

 	int *threads = (int*)malloc(10*sizeof(int));
 	int i;


 	if(threads==NULL)
 		return 0;
 	for(i=0;i<10;i++){
 		threads[i]=mcreate(i%3, (void*)quem_sou_eu, void);
 	}
 	mwait(3);





 	printf("\nTérmino da main... TID = %i\n", executing_thread->tid);
 	return 0;
 }
