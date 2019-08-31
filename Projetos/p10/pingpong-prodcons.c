#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"

// valores dos semaforos
#define SEM_VAL_ITEM 0
#define SEM_VAL_VAGA 5
#define SEM_VAL_BUFFER 5

task_t p1, p2, p3, c1, c2;
semaphore_t s_item, s_buffer, s_vaga;

int buffer[5]; // fila de 5 posicoes
int curr_pos = 0;
int item; // valor entre 0 e 99

void produtor(void * arg)
{
    while(1)
    {
        task_sleep (1);
        item = rand()%100;
        sem_down (&s_vaga);
        sem_down (&s_buffer);
        // insere item no buffer
        buffer[curr_pos] = item;
        curr_pos++;
        printf("%s produziu %d\n", (char*)arg, item);
        sem_up (&s_buffer);
        sem_up (&s_item);
    }
}

void consumidor (void * arg)
{
    while(1)
    {
        sem_down (&s_item);
        sem_down (&s_buffer);
        // retira item do buffer
        curr_pos--;
        printf("%s consumiu %d\n", (char*)arg, buffer[0]);
        int i;
        for(i = 0; i < curr_pos; i++)
        {
            buffer[i] = buffer[i+1];
        }
        sem_up (&s_buffer);
        sem_up (&s_vaga);
        task_sleep (1);
    }
}

int main (int argc, char *argv[])
{
    pingpong_init () ;
    sem_create(&s_item, SEM_VAL_ITEM);
    sem_create(&s_buffer, SEM_VAL_BUFFER);
    sem_create(&s_vaga, SEM_VAL_VAGA);
    
    task_create(&p1, produtor, "p1");
    task_create(&c1, consumidor, "                  c1");
    task_create(&p2, produtor, "p2");
    task_create(&p3, produtor, "p3");
    task_create(&c2, consumidor, "                  c2");

    // para main nao sair
    task_join(&p1);
    task_exit (0) ;

    exit (0) ;
}
