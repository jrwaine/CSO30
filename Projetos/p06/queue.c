#include "queue.h"
#include <stdio.h> // para msgs de erro

void queue_append (queue_t **queue, queue_t *elem)
{
    if(elem == NULL)
    {
        printf("Elemento nao existe, nao eh possivel inseri-lo!\n");
        return;
    }
    if(elem->next != NULL || elem->prev != NULL)
    {
        printf("Elemento em outra fila, nao eh possivel inseri-lo!\n");
        return;
    }
    if(queue == NULL)
    {
        printf("Fila nao existe, nao eh possivel inserir elemento!\n");
        return;
    }
    else if((*queue) == NULL) // fila vazia
    {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return;
    }

    queue_t* aux = (*queue)->prev;
    elem->next = *queue;
    (*queue)->prev = elem;
    elem->prev = aux;
    aux->next = elem;
}

queue_t* queue_remove(queue_t **queue, queue_t *elem)
{
    if(elem == NULL)
    {
        printf("Elemento nao existe, nao eh possivel remove-lo!\n");
        return NULL;
    }
    if(queue == NULL)
    {
        printf("Fila nao existe, nao eh possivel remover elemento!\n");
        return NULL;
    }
    if((*queue) == NULL)
    {
        printf("Fila vazia, nao eh possivel remover elemento!\n");
        return NULL;
    }
    
    // fila de um elemento
    if(elem == *queue && elem->next == elem && elem->prev == elem)
    {
        elem->next = NULL;
        elem->prev = NULL;
        *queue = NULL;
        return elem;
    }

    queue_t* aux = (*queue);
    do{
        if(aux == elem)
        {
            // remove elemento
            elem->next->prev = elem->prev;
            elem->prev->next = elem->next;
            if(elem == *queue)
                *queue = elem->next;
            elem->next = NULL;
            elem->prev = NULL;
            return elem;
        }
        aux = aux->next;
    }while(aux != *queue);

    printf("Elemento nao esta na fila, nao eh possivel remove-lo!\n");
    return NULL;
}


int queue_size (queue_t *queue)
{
    if(queue == NULL)
        return 0;
        
    queue_t* aux = queue;
    int cont = 1;

    for(aux = queue; aux->next != queue; aux = aux->next)
        cont++;
    return cont;
}


void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
    printf("Fila %s\n", name);
    if(queue == NULL)
    {
        printf("Fila vazia!\n");
        return;
    }
    queue_t* aux;
    print_elem(queue);
    printf(" ");
    for(aux = queue->next; aux != queue; aux = aux->next)
    {    
        print_elem(aux);
        printf(" ");
    }
    printf("\n");
}
