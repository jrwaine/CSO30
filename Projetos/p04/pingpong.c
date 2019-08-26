#include "datatypes.h"
#include "pingpong.h"

unsigned int __tid = 0;
task_t __task_main;
task_t __task_dispatcher;
task_t* __curr_task;
// Fila para as tarefas que nao sao do usuario criadas
struct queue_t* __queue_init_tasks = NULL;
// Fila para as tarefas prontas
struct queue_t* __queue_ready_tasks = NULL;
// Fila para as tarefas suspensas
struct queue_t* __queue_susps_tasks = NULL;
// Fila para as tarefas finalizadas (atualizada pelo task_exit)
struct queue_t* __queue_ended_tasks = NULL;

void update_queues(task_t* task, int new_state);
void dispatcher();
task_t* scheduler();
int task_getprio_total(task_t* task);

// Atualiza filas e tarefa para um novo estado (init, pronta, suspensa)
void update_queues(task_t* task, int new_state)
{
    queue_t** queue_rm, **queue_add;
    if(task == NULL)
        return;
    if(task->state == INIT)
        queue_rm = &__queue_init_tasks;
    else if(task->state == READY)
        queue_rm = &__queue_ready_tasks;
    else if(task->state == SUSPS)
        queue_rm = &__queue_susps_tasks;
    else if(task->state == ENDED)
        queue_rm = &__queue_ended_tasks;

    if(new_state == INIT)
        queue_add = &__queue_init_tasks;
    else if(new_state == READY)
        queue_add = &__queue_ready_tasks;
    else if(new_state == SUSPS)
        queue_add = &__queue_susps_tasks;
    else if(new_state == ENDED)
        queue_add = &__queue_ended_tasks;
    
    queue_remove((queue_t**)(queue_rm), (queue_t*)(task));
    queue_append((queue_t**)(queue_add), (queue_t*)(task));
}

// Retorna a soma da prioridade dinamica com a estatica da tarefa
int task_getprio_total(task_t* task)
{
    return (task->stc_prior+task->dyn_prior);
}

// Despachador
void dispatcher()
{
    // Enquanto ainda houver tarefas criadas pelo usuario
    // Obs.: fila deve ser maior que um pois a tarefa do dispatcher esta nela
    while(queue_size((queue_t*)(__queue_ready_tasks)) > 1)
    {
        // pega a próxima tarefa
        task_t* next = scheduler();

        if(next != &__task_dispatcher)
        {
            task_switch(next);
        }
    }
    task_exit(0);
}

// Escalonador
task_t* scheduler()
{
    task_t* task = __task_dispatcher.next;
    task_t* aux;
    // Tarefa com maior prioridade (menor valor)
    for(aux = task->next; aux != &__task_dispatcher; aux = aux->next)
    {
        if(task_getprio_total(aux) < task_getprio_total(task))
            task = aux;
    }
    
    // Atualizar prioridade das tarefas
    for(aux = __task_dispatcher.next; aux != &__task_dispatcher; aux = aux->next)
    {
        if(aux != task)
        {
            aux->dyn_prior += ALPHA_PRIOR;
            // Validar prioridade
            if(task_getprio_total(aux) > HIGHEST_PRIOR)
                aux->dyn_prior = HIGHEST_PRIOR - aux->stc_prior;
            else if(task_getprio_total(aux) < LOWEST_PRIOR)
                aux->dyn_prior = LOWEST_PRIOR - aux->stc_prior;
        }
    }
    // Reseta a prioridade dinâmica da tarefa escalonada
    task->dyn_prior = 0;
    return task;
}


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
    __task_main.state = READY;
    __task_main.stc_prior = DEFAULT_PRIOR;
    __task_main.dyn_prior = 0;
    __tid++;
    
    // Adiciona a task main à fila de tasks
    // queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(&__task_main));
    
    // Cria a tarefa dispatcher
    task_create(&__task_dispatcher, (void*)(*dispatcher), NULL);

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
    task->state = READY; // por padrao a tarefa esta pronta para execucao
    queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(task));
    task->stc_prior = DEFAULT_PRIOR;
    task->dyn_prior = 0;
    __tid++;
    
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
    update_queues(__curr_task, ENDED);
    if(__curr_task == &__task_dispatcher) // exit tarefas do user é p dispatcher
        task_switch(&__task_main);
    else
        task_switch(&__task_dispatcher);
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


void task_suspend (task_t *task, task_t **queue)
{
    if(queue == NULL)
    {
        return;
    }
    if(task == NULL)
    {
        task = __curr_task;
    }
    update_queues(task, SUSPS);
}


void task_resume (task_t *task)
{
    update_queues(task, READY);
}


void task_yield ()
{
    if(__curr_task != &__task_main) // main nao esta em nenhuma fila
    {
        //queue_remove((queue_t**)(&__curr_task), (queue_t*)__curr_task);
        //queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)__curr_task);
        update_queues(__curr_task, READY);
    }
    task_switch(&__task_dispatcher);
}


void task_setprio (task_t *task, int prio)
{
    if(task == NULL)
        task = __curr_task;
    if(prio < LOWEST_PRIOR)
        task->stc_prior = LOWEST_PRIOR;
    else if(prio > HIGHEST_PRIOR)
        task->stc_prior = HIGHEST_PRIOR;
    else
        task->stc_prior = prio;
}


int task_getprio (task_t *task)
{
    if(task == NULL)
        task = __curr_task;
    return task->stc_prior;
}
