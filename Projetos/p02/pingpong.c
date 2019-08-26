#include "datatypes.h"
#include "pingpong.h"

unsigned int __tid = 0;
task_t __task_main;
task_t* __curr_task;
struct queue_t* __queue_task;

void pingpong_init()
{
#ifdef DEBUG
    printf("Inicializando Ping Pong OS\n");
#endif
    // desativa o buffer da saida padrao (stdout), usado pela função printf
    setvbuf (stdout, 0, _IONBF, 0); // ARRUMAR

    // salva contexto atual na main
    getcontext(&__task_main.context);
    char* stack = malloc (SIGSTKSZ) ;
    if (stack)
    {
        __task_main.context.uc_stack.ss_sp = stack ;
        __task_main.context.uc_stack.ss_size = SIGSTKSZ;
        __task_main.context.uc_stack.ss_flags = 0;
        __task_main.context.uc_link = 0; 
    }
    else
    {
        perror ("Erro na criação da pilha da tarefa main\n");
        exit(-1);
    }
    __task_main.prev = NULL;
    __task_main.next = NULL;
    __task_main.tid = __tid;
    __tid++;
    __queue_task = NULL;
    // Adiciona a task main à fila de tasks
    queue_append((queue_t**)(&__queue_task), (queue_t*)(&__task_main));
    __curr_task = &(__task_main);
}


int task_create (task_t *task,
    void (*start_func)(void *),
    void *arg)
{
#ifdef DEBUG
    printf("Criando tarefa de ID %d\n", __tid);
#endif
    // Relacionado a task
    task->prev = NULL;
    task->next = NULL;
    task->tid = __tid;
    __tid++;
    queue_append((queue_t**)(&__queue_task), (queue_t*)(task));
    
    // Relacionado a criacao e configuracao do contexto
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
#ifdef DEBUG
    printf("Encerrando tarefa de ID %d com codigo %d\n", __curr_task->tid, exitCode);
#endif
    task_switch(&__task_main);
}


int task_switch (task_t *task)
{
    if(task == NULL)
    {
        printf("Tarefa nao inicializada, nao eh possivel trocar para ela\n");
        return -1;
    }
    if(task == __curr_task)
    {
        printf("Tarefa ja em execucao, nao eh possivel trocar para ela\n");
        return -1;
    }
#ifdef DEBUG
    printf("Tarefa de ID %d assumindo o processador\n", task->tid);
#endif
    // atualiza task atual e muda de contextos
    task_t* aux = __curr_task;
    __curr_task = task;
    return swapcontext(&(aux->context), &(task->context));
}


int task_id ()
{
#ifdef DEBUG
    printf("Pegando o ID da tarefa atual\n");
#endif
    return __curr_task->tid;
}
