#include <stdio.h>
#include <stdlib.h>
#include "../include/mthread.h"

void *oi()
{
    i=0;
    while(i<5)
    {
        printf("oi\n");
        i++;
        if(i==4)
        {
            myield();
        }
    }
}



void *tchau()
{
    printf("tchau\n");
}

int main ()
{
    int id1,id2, id3;

    id1=mcreate(0, &oi, void);
    id2=mcreate(0, &tchau, void);
    id3=mcreate(0, &tchau, void);


    printf("Uma sequencia de cumprimentos:\n");

    return 0;
}
