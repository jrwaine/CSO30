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
#include <signal.h> // para tamanho da stack e signals
#include <sys/time.h> // para interrupcoes por tempo
#include <stdio.h>  // para buffer do printf
#include <stdlib.h> // para malloc
#include <strings.h> // para bcopy

// defines relacionados a task
#define DISK_REQUEST 0
#define READY 1
#define SUSPS 2
#define ENDED 3
#define SLEEP 4

// defines para prioridade
#define LOWEST_PRIOR (-20)
#define HIGHEST_PRIOR (+20)
#define DEFAULT_PRIOR (0)
#define ALPHA_PRIOR (-1)

// defines para ticks
#define TICK_INTERVAL (1000) // em microssegundos
#define TICK_QUANTUM (20) // numero de ticks por tarefa

// defines para classificacoes das tarefas
#define TASK_SYS 0
#define TASK_USER 1

// imprimir informacoes de tempo das tarefas, comentar caso n desejado
#define PRINT_INFO

// numero maximos de tarefas que podem depender de uma unica tarefa
#define MAX_TASKS_JOIN 20

// numero de ticks para chamar watcher de tarefas dormindo
#define TICKS_SLEEP_WATCHER 100

// defines relacionados ao semaforo
#define SEM_INITIALIZED 0
#define SEM_DESTROYED 1

// defines relacionados a barreira
#define BARR_INITIALIZED 0
#define BARR_DESTROYED 1

// defines relacionados a fila de mensagens
#define MQUEUE_INITIALIZED 0
#define MQUEUE_DESTROYED 1


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
    // Numero de ticks restantes da tarefa
    int ticks;
    // Classificador do tipo da tarefa
    int task_type;
    
    // Contadores de ticks (timers) da tarefa, relacionados a TICK_INTERVAL
    unsigned int time_ini;      // atualiza quando tarefa eh criada
    unsigned int time_total;    // atualiza quando perde ou ganha cpu
    unsigned int time_cpu;      // atualiza quando perde cpu
    unsigned int time_last_actv;// atualiza quando ganha cpu
    // Numero de ativacoes
    unsigned int n_activations; // atualiza quando ganha cpu
    // Tarefa ja foi iniciada ou nao
    int is_ini;                 // atualiza quando ganha cpu pela 1 vez

    // Vetor de tarefas que dependem dessa
    struct task_t* queue_tasks_join[MAX_TASKS_JOIN];
    // Numero de tarefas que dependem dessa
    unsigned int n_tasks_join;
    // Codigo de saida da tarefa
    int exit_code;

    // Tempo que a tarefa comecou a dormir
    unsigned int time_ini_sleep;
    // Tempo que a tarefa pode acordar
    unsigned int time_wake;

    // Operacao requisitada ao disco (read ou write)
    int disk_oper;
    // bloco a ser lido ou escrito
    int disk_block;
    // buffer para ser escrito ou carregado
    void* disk_buffer;

} task_t ;

// estrutura que define um semáforo
typedef struct
{
    int value;
    task_t* queue_sem;
    int state;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
    // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
    task_t* queue_barrier;
    int N;
    int n_threads;
    int state;
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
    // fila de mensagens
    void** queue_msg;
    // semaforo para tarefas esperando para receber msg
    semaphore_t sem_rcv;
    // semaforo para tarefas esperando para enviar msg
    semaphore_t sem_send;
    // numero maximo de mensagens
    int max_n_msg; 
    // tamanho maximo de mensagem
    int max_size_msg;
    // numero atual de mensagens
    int n_msg;
    // estado da fila
    int state;
} mqueue_t ;

#endif
