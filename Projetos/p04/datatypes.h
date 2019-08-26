// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#define _XOPEN_SOURCE 600   /* Para compilar no MAC */
#include <ucontext.h>
#include "queue.h"
#include <signal.h> // para tamanho da stack
#include <stdio.h>  // para buffer do printf
#include <stdlib.h> // para malloc

// defines relacionados a task
#define INIT 0
#define READY 1
#define SUSPS 2
#define ENDED 3

// defines for priority
#define LOWEST_PRIOR (-20)
#define HIGHEST_PRIOR (+20)
#define DEFAULT_PRIOR (0)
#define ALPHA_PRIOR (-1)

// Estrutura que define uma tarefa
typedef struct task_t
{
    // Cast para queue_t para operações
    struct task_t *prev;
    struct task_t *next;
    // ID da tarefa
    int tid;
    // Contexto da tarefa
    ucontext_t context;
    // Estado da tarefa
    int state;
    // Prioridade estática da tarefa (escala negativa)
    int stc_prior;
    // Prioridade dinâmica da tarefa (escala negativa)
    int dyn_prior;
} task_t ;

// estrutura que define um semáforo
typedef struct
{
    // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
    // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
    // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
    // preencher quando necessário
} mqueue_t ;

#endif
