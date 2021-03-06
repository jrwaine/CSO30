#include "datatypes.h"
#include "pingpong.h"

unsigned int __tid = 0;
task_t __task_main;
task_t __task_dispatcher;
task_t* __curr_task;
int __not_preempt = 0;

// Fila para as tarefas que nao sao do usuario criadas
struct queue_t* __queue_init_tasks = NULL;
// Fila para as tarefas prontas
struct queue_t* __queue_ready_tasks = NULL;
// Fila para as tarefas suspensas
struct queue_t* __queue_susps_tasks = NULL;
// Fila para as tarefas finalizadas (atualizada pelo task_exit)
struct queue_t* __queue_ended_tasks = NULL;

// Tratador de sinal
struct sigaction action ;
// Inicialização to timer
struct itimerval timer;


void update_queues(task_t* task, int new_state);
void dispatcher();
task_t* scheduler();
int task_getprio_total(task_t* task);
void sig_treat();


// Atualiza filas e tarefa para um novo estado (init, pronta, suspensa)
void update_queues(task_t* task, int new_state)
{
    int aux_preempt = __not_preempt;
    __not_preempt = 1;
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
    __not_preempt = aux_preempt;
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
        task_switch(next);

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

void sig_treat()
{
#ifdef DEBUG
    printf("Tick recebido, tarefa atual possui %d ticks", __curr_task->ticks);
#endif
    // nao preempta tarefas de sistema
    if(__curr_task->task_type == TASK_SYS)
        return;
    if(__curr_task->ticks > 0)
        __curr_task->ticks--;
    
    if(__curr_task->ticks <= 0)
    {
        // preempta e retorna para o dispatcher
        if(!__not_preempt)
            task_yield(&__task_dispatcher);
    }
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
    __task_main.ticks = TICK_QUANTUM;
    __task_main.task_type = TASK_USER;
    __tid++;
    // task atual eh a main
    __curr_task = &(__task_main);
    
    // Adiciona a task main à fila de tasks
    // queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(&__task_main));
    
    // Cria a tarefa dispatcher
    task_create(&__task_dispatcher, (void*)(*dispatcher), NULL);
    __task_dispatcher.task_type = TASK_SYS;

    // Configura tratador de sinais
    action.sa_handler = sig_treat;
    sigemptyset (&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // Configura temporizador
    timer.it_value.tv_usec = 100;      // primeiro disparo, em microssegundos
    timer.it_interval.tv_usec = TICK_INTERVAL; // disparos subsequentes, em microssegundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}


int task_create (task_t *task,
    void (*start_func)(void *),
    void *arg)
{
#ifdef DEBUG
    printf("Criando tarefa de ID %d\n", __tid);
#endif
    __not_preempt = 1;
    // Relacionado a task
    task->prev = NULL;
    task->next = NULL;
    task->tid = __tid;
    task->state = READY; // por padrao a tarefa esta pronta para execucao
    queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(task));
    task->stc_prior = DEFAULT_PRIOR;
    task->dyn_prior = 0;
    task->ticks = TICK_QUANTUM;
    task->task_type = TASK_USER;
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
    __not_preempt = 0;
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
    __not_preempt = 1;
    task_t* aux = __curr_task;
    __curr_task = task;
    __curr_task->ticks = TICK_QUANTUM;
    __not_preempt = 0;
    return swapcontext(&(aux->context), &(task->context));
}


int task_id ()
{
#ifdef DEBUG
    printf("Pegando o ID da tarefa atual\n");
#endif
    return __curr_task->tid;
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
