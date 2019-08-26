#include "datatypes.h"
#include "pingpong.h"

unsigned int __tid = 0;
task_t __context_main;
task_t* __curr_task;
struct queue_t* __queue_task;

#define DEBUG

void pingpong_init()
{
#ifdef DEBUG
    printf("Inicializando Ping Pong OS\n");
#endif
    // desativa o buffer da saida padrao (stdout), usado pela função printf
    setvbuf (stdout, 0, _IONBF, 0); // ARRUMAR

    // salva contexto atual na main
    getcontext(&__context_main.context);
    __context_main.tid = __tid;
    __tid++;
    __queue_task = NULL;
    // Adiciona a task main à fila de tasks
    queue_append((queue_t**)(&__queue_task), (queue_t*)(&__context_main));
    __curr_task = &(__context_main);
}


int task_create (task_t *task,
    void (*start_func)(void *),
    void *arg)
{
#ifdef DEBUG
    printf("Criando tarefa de ID %d\n", __tid);
#endif
    // Relacionado a task
    task->tid = __tid;
    __tid++;
    queue_append((queue_t**)(&__queue_task), (queue_t*)(task));
    
    // Relacionado a criação e configuração do contexto
    getcontext(&(task->context));
    char* stack = malloc (SIGSTKSZ) ;
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack ;
        task->context.uc_stack.ss_size = SIGSTKSZ;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0; 
    }
    else
    {
        perror ("Erro na criação da pilha da tarefa\n");
        return -1;
    }
    makecontext(&(task->context), (void*)(*start_func), 1, arg);

    return task->tid;
}


void task_exit (int exitCode)
{
    task_switch(&__context_main);
}


int task_switch (task_t *task)
{
    if(task == NULL)
    {
        printf("Tarefa nao inicializada, nao eh possivel trocar para ela\n");
        return -1;
    }
#ifdef DEBUG
    printf("Tarefa de ID %d assumindo o processador\n", task->tid);
#endif    
    int i = swapcontext(&(__curr_task->context), &(task->context));
    __curr_task = task;
    return i;
}


int task_id ()
{
#ifdef DEBUG
    printf("Pegando o ID da tarefa atual\n");
#endif
    return __curr_task->tid;
}
