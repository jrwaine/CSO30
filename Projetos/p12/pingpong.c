#include "datatypes.h"
#include "pingpong.h"

// TODO: ALTERAR CODIGO PARA EVITAR QUE PREEMPCAO NO MEIO DE UPDATE QUEUES,
// EM TAREFAS COMO TASK_SLEEP E TASK_JOIN CAUSEM PROBLEMAS NO SO

unsigned int __tid = 0;
task_t __task_main;
task_t __task_dispatcher;
task_t* __curr_task;
int __not_preempt = 0;

// Fila inutilizada
struct queue_t* __queue_init_tasks = NULL;
// Fila para as tarefas prontas
struct queue_t* __queue_ready_tasks = NULL;
// Fila para as tarefas suspensas (atualizada pelo task_join)
struct queue_t* __queue_susps_tasks = NULL;
// Fila para as tarefas finalizadas (atualizada pelo task_exit)
struct queue_t* __queue_ended_tasks = NULL;
// Fila para tarefas dormindo (atualizada pelo task_sleep)
struct queue_t* __queue_sleep_tasks = NULL;

// Tratador de sinal
struct sigaction __action ;
// Inicialização to timer
struct itimerval __timer;
// Numero total de ticks
unsigned int __total_ticks = 0;
// Ticks para chamar watcher das tarefas dormindo (0 a WATCHER_SLEEP)
unsigned int __sleep_ticks = 0;


void update_queues(task_t* task, int new_state);
void dispatcher();
task_t* scheduler();
int task_getprio_total(task_t* task);
void sig_treat();
unsigned int systime();
void print_task_info(task_t* task);
void sleep_watcher();


// Atualiza filas e tarefa para um novo estado (init, pronta, suspensa)
void update_queues(task_t* task, int new_state)
{
#ifdef DEBUG
    printf("Tarefa %d trocando estado %d para %d\n", task->tid, task->state, new_state);
#endif
    int aux = __not_preempt;
    // nao pode haver preempcao na atualizacao de filas por possiveis
    // inconsistencia nas filas
    __not_preempt = 1;

    queue_t **queue_add, **queue_rm;
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
    else if(task->state == SLEEP)
        queue_rm = &__queue_sleep_tasks;
    
    if(new_state == INIT)
        queue_add = &__queue_init_tasks;
    else if(new_state == READY)
        queue_add = &__queue_ready_tasks;
    else if(new_state == SUSPS)
        queue_add = &__queue_susps_tasks;
    else if(new_state == ENDED)
        queue_add = &__queue_ended_tasks;
    else if(new_state == SLEEP)
        queue_add = &__queue_sleep_tasks;
    
    task->state = new_state;
    
    if(*queue_rm != NULL)
        queue_remove((queue_t**)(queue_rm), (queue_t*)(task));
    queue_append((queue_t**)(queue_add), (queue_t*)(task));
    __not_preempt = aux;
}

// Retorna a soma da prioridade dinamica com a estatica da tarefa
int task_getprio_total(task_t* task)
{
    return (task->stc_prior+task->dyn_prior);
}

// Despachador
void dispatcher()
{
    // Enquanto ainda houver tarefas prontas e nem todas tiverem terminado 
    // (sem contar dispatcher)
    // Obs.: fila deve ser maior que um pois a tarefa do dispatcher esta nela
    while(queue_size((queue_t*)(__queue_ready_tasks)) > 1 
        || queue_size(__queue_ended_tasks) < ((int)__tid-1)
        || queue_size(__queue_susps_tasks) > 0)
    {
        // De tempos em tempos chama o sleep_watcher
        if(__sleep_ticks >= TICKS_SLEEP_WATCHER)
            sleep_watcher();
        // Pega a próxima tarefa
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
#ifdef DEBUG
    printf("Pegando proxima tarefa\n");
#endif

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


// Tratador de sinal SIGALRM (timer/alarme)
void sig_treat()
{
#ifdef DEBUG
    printf("Tick, tarefa %d possui %d ticks\n", __curr_task->tid, __curr_task->ticks);
#endif
    __total_ticks++;
    if(__sleep_ticks < TICKS_SLEEP_WATCHER)
        __sleep_ticks++;
    
    if(__curr_task->ticks > 0)
        __curr_task->ticks--;
    
    if(__not_preempt)
        return;

    // nao preempta tarefas de sistema
    if(__curr_task->task_type == TASK_SYS)
        return;
    if(__curr_task->ticks <= 0)
    {
        // preempta e retorna para o dispatcher
        __curr_task->ticks = TICK_QUANTUM;
        task_yield(&__task_dispatcher);
    }
}


// "Clock" do SO
unsigned int systime()
{
    return __total_ticks;
}


// Imprime informacoes de tempo da tarefa em sua saida
void print_task_info(task_t* task)
{
    printf("Task %d exit: ", task_id(task));
    printf("execution time %d ms, ", task->time_total);
    printf("processor time %d ms, ", task->time_cpu);
    printf("%d activations\n", task->n_activations);
}


// Acorda tarefas que ja podem acordar
void sleep_watcher()
{
#ifdef DEBUG
    printf("Checando tarefas dormindo\n");
#endif

    int size = queue_size(__queue_sleep_tasks);
    // tasks que devem continuar dormindo
    int cont = 0;
    task_t* task = (task_t*)__queue_sleep_tasks;

    // se ha tarefas dormindo
    while(size > cont)
    {
        // se a tarefa ja pode acordar
        if(systime() >= task->time_wake)
        {
            task = task->next;
            update_queues(task->prev, READY);
        }
        else // se n pode vai para prox
        {
            task = task->next;
        }
        cont++;
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

    __task_main.time_ini = 0;
    __task_main.time_total = 0;
    __task_main.time_cpu = 0;
    __task_main.time_last_actv = 0;
    __task_main.n_activations = 0;
    
    __task_main.is_ini = 0;
    int i;
    for(i = 0; i < MAX_TASKS_JOIN; i++)
        __task_main.queue_tasks_join[i] = NULL;
    __task_main.n_tasks_join = 0;
    
    __task_main.time_ini_sleep = 0;
    __task_main.time_wake = 0;
    
    __tid++;


    // task atual eh a main
    __curr_task = &(__task_main);
    
    // Cria a tarefa dispatcher
    task_create(&__task_dispatcher, (void*)(*dispatcher), NULL);
    __task_dispatcher.task_type = TASK_SYS; // atualiza tipo de dispatcher

    // Configura tratador de sinais
    __action.sa_handler = sig_treat;
    sigemptyset (&__action.sa_mask);
    __action.sa_flags = 0;
    if (sigaction (SIGALRM, &__action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // Configura temporizador
    __timer.it_value.tv_usec = 100;      // primeiro disparo, em microssegundos
    __timer.it_interval.tv_usec = TICK_INTERVAL; // disparos subsequentes, em microssegundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &__timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }

    // atualiza tempo da main
    __task_main.time_ini = systime(); 
    
    // coloca main na fila de tarefas prontas (apos o dispatcher)
    update_queues(&__task_main, READY);

    // inicia o dispatcher
    task_yield(&__task_dispatcher);
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
    
    task->stc_prior = DEFAULT_PRIOR;
    task->dyn_prior = 0;
    task->ticks = TICK_QUANTUM;
    task->task_type = TASK_USER;

    task->time_total = 0;
    task->time_cpu = 0;
    task->time_last_actv = 0;
    task->n_activations = 0;
    task->is_ini = 0;

    int i;
    for(i = 0; i < MAX_TASKS_JOIN; i++)
        task->queue_tasks_join[i] = NULL;
    task->n_tasks_join = 0;
    
    task->time_ini_sleep = 0;
    task->time_wake = 0;

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
    
    // adiciona tarefa a fila de prontas
    update_queues(task, READY);
    // atualiza tempo inicial da tarefa
    task->time_ini = systime();

    return task->tid;
}


void task_exit (int exitCode)
{
#ifdef DEBUG
    printf("Encerrando tarefa de ID %d com codigo %d\n", __curr_task->tid, exitCode);
#endif
    update_queues(__curr_task, ENDED);
    __curr_task->exit_code = exitCode;

    // disponibiliza as outras tarefas que dependem dessa para execucao
    while(__curr_task->n_tasks_join > 0)
    {
        __curr_task->n_tasks_join--;
        task_t* aux = __curr_task->queue_tasks_join[__curr_task->n_tasks_join];
        update_queues(aux, READY);
    }
    // saida de tarefas eh sempre para o dispatcher
    task_switch(&__task_dispatcher);
}


int task_switch (task_t *task)
{
    if(task == NULL)
    {
        printf("Tarefa nao inicializada, nao eh possivel trocar para ela\n");
        return -1;
    }
#ifdef DEBUG
    printf("Trocando de tarefa de %d para %d\n", __curr_task->tid, task->tid);
#endif
    if(!(task->is_ini))
    {
        task->is_ini = 1;
    }
    int aux_preempt = __not_preempt;
    // nao preempta durante o switch
    __not_preempt = 1;
    // atualiza timers da tarefa atual
    __curr_task->time_cpu += systime() - __curr_task->time_last_actv;
    __curr_task->time_total = systime() - __curr_task->time_ini;
    
#ifdef PRINT_INFO
    // imprime informacoes caso a tarefa tenha sido terminada
    if(__curr_task->state == ENDED)
    {
        print_task_info(__curr_task);
    }
#endif

    // atualiza timers da tarefa que ira ganhar cpu
    task->time_last_actv = systime();
    task->time_total = systime() - task->time_ini;
    task->n_activations++;

    // atualiza task atual e muda de contextos
    task_t* aux = __curr_task;
    __curr_task = task;
    __curr_task->ticks = TICK_QUANTUM;
    __not_preempt = aux_preempt;
    if(task != aux)
        return swapcontext(&(aux->context), &(task->context));
    return 0;
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
#ifdef DEBUG
    printf("Preemptando tarefa %d\n", __curr_task->tid);
#endif
    update_queues(__curr_task, READY);
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


int task_join (task_t *task)
{
    // se task nao existe, retorna erro (-1)
    if(task == NULL)
        return -1;
    // se task ja foi finalizada, retorna seu codigo de saida
    if(task->state == ENDED)
        return task->exit_code;
#ifdef DEBUG
    printf("Tarefa %d dependendo de %d (join)\n", __curr_task->tid, task->tid);
#endif
    // coloca a tarefa atual no vetor de dependencias de task
    task->queue_tasks_join[task->n_tasks_join] = __curr_task;
    task->n_tasks_join++;
    // tira a tarefa atual da fila de prontas
    update_queues(__curr_task, SUSPS);
    // chama escalonador
    task_switch(&__task_dispatcher);

    // garante que tarefa foi finalizada
    if(task->exit_code == ENDED)
        // retorna codigo de saida da task
        return task->exit_code;
    return -1;
}

void task_sleep (int t)
{
#ifdef DEBUG
    printf("Tarefa %d dorme por %d segundos\n", __curr_task->tid, t);
#endif
    // converte t de segundos para ticks (tick = 1ms)
    int ticks = t*1000;
    // atualiza novos valores da tarefa
    __curr_task->time_ini_sleep = systime();
    __curr_task->time_wake = __curr_task->time_ini_sleep + ticks;
    // atualiza fila de tarefas e muda para dispatcher
    update_queues(__curr_task, SLEEP);
    task_switch(&__task_dispatcher);
}


int sem_create (semaphore_t *s, int value)
{
    if(s == NULL)
        return -1;
#ifdef DEBUG
    printf("Criando semaforo com valor %d\n", value);
#endif
    int aux_preempt = __not_preempt;
    // tarefas de semaforo nao podem ser preemptadas
    __not_preempt = 1;
    
    s->value = value;
    s->queue_sem = NULL;
    s->state = SEM_INITIALIZED;
    
    __not_preempt = aux_preempt;
    
    return 0;
}


int sem_down (semaphore_t *s)
{
    if(s == NULL)
        return -1;
    if(s->state == SEM_DESTROYED)
        return -1;
#ifdef DEBUG
    printf("Semaforo down, valor %d, tarefa curr %d\n", s->value-1, __curr_task->tid);
#endif
    int aux_preempt = __not_preempt;
    // tarefas de semaforo nao podem ser preemptadas
    __not_preempt = 1;
    
    // decrementa contador do semaforo
    s->value--;
    
    // caso contador seja negativo
    if(s->value < 0)
    {
        // retira tarefa atual da fila de prontas 
        //(todas tarefas executando estao na fila de prontas)
        queue_remove((queue_t**)(&__queue_ready_tasks), (queue_t*)__curr_task);
        // adiciona tarefa atual na fila do semaforo
        queue_append((queue_t**)(&(s->queue_sem)), (queue_t*)(__curr_task));
        // volta ao dispatcher
        __not_preempt = aux_preempt;
        task_switch(&__task_dispatcher);
        
        // checa se o semaforo foi destruido ao retornar a execucao da tarefa
        if(s->state == SEM_DESTROYED)
            return -1;
    }

    __not_preempt = aux_preempt;

    return 0;
}


int sem_up (semaphore_t *s)
{
    if(s == NULL)
        return -1;
    if(s->state == SEM_DESTROYED)
        return -1;

#ifdef DEBUG
    printf("Semaforo up, valor %d, tarefa curr %d\n", s->value+1, __curr_task->tid);
#endif
    int aux_preempt = __not_preempt;
    // tarefas de semaforo nao podem ser preemptadas
    __not_preempt = 1;
    
    // aumenta valor do semaforo
    s->value++;
    // se fila do semaforo nao esta vazia
    task_t* task = s->queue_sem;
    if(task != NULL)
    {
        // remove primeira tarefa da fila e adiciona a fila de prontas
        queue_remove((queue_t**)(&s->queue_sem), (queue_t*)(task));
        queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(task));
        task->ticks = TICK_QUANTUM;
    }
    
    __not_preempt = aux_preempt;
    
    return 0;
}


int sem_destroy (semaphore_t *s)
{
    if(s == NULL)
        return -1;
    if(s->state == SEM_DESTROYED)
        return -1;
#ifdef DEBUG
    printf("Destruindo semaforo com valor %d\n", s->value); fflush(stdout);
#endif
    int aux_preempt = __not_preempt;
    // tarefas de semaforo nao podem ser preemptadas
    __not_preempt = 1;

    // remove todas tarefas do semaforo e adiciona a fila de prontas
    while(queue_size((queue_t*)(s->queue_sem)) > 0)
    {
        task_t* task = s->queue_sem;
        // remove primeira tarefa da fila e adiciona a fila de prontas
        queue_remove((queue_t**)(&s->queue_sem), (queue_t*)(task));
        queue_append((queue_t**)(&__queue_ready_tasks), (queue_t*)(task));
    }
    s->state = SEM_DESTROYED;

    __not_preempt = aux_preempt;

    return 0;
}


int barrier_create (barrier_t *b, int N)
{
    if(b == NULL)
        return -1;
    
    int aux_preempt = __not_preempt;
    // tarefas de barreira nao podem ser preemptadas
    __not_preempt = 1;
    
    b->N = N;
    b->n_threads = 0;
    b->queue_barrier = NULL;
    b->state = BARR_INITIALIZED;
    
    __not_preempt = aux_preempt;
    return 0;
}


int barrier_join (barrier_t *b)
{
    if(b == NULL)
        return -1;

    int aux_preempt = __not_preempt;
    // tarefas de barreira nao podem ser preemptadas
    __not_preempt = 1;
    
    b->n_threads++;
    // se o numero de tarefas foi alcancado
    if(b->n_threads >= b->N)
    {
        // enquanto houver ainda tarefas que dependem dessa
        while(b->queue_barrier != NULL)
        {
            // retira tarefa da fila da barreira e adiciona a fila de prontas
            // ou seja, libera as tarefas
            task_t* task = b->queue_barrier;
            queue_remove((queue_t**)&b->queue_barrier, (queue_t*)task);
            queue_append((queue_t**)&__queue_ready_tasks, (queue_t*)task);
            b->n_threads--;
        }
    }
    else 
    {
        // adiciona a tarefa para espera na fila da barreira
        queue_remove((queue_t**)&__queue_ready_tasks, (queue_t*)__curr_task);
        queue_append((queue_t**)&b->queue_barrier, (queue_t*)__curr_task);
        __not_preempt = aux_preempt;
        task_switch(&__task_dispatcher);
        // checa se a barreira nao foi destruida durante a espera

    }
    
    __not_preempt = aux_preempt;
    return 0;
}


int barrier_destroy (barrier_t *b)
{
    if(b == NULL)
        return -1;

    int aux_preempt = __not_preempt;
    // tarefas de barreira nao podem ser preemptadas
    __not_preempt = 1;

    // enquanto houver ainda tarefas que dependem dessa
    while(b->queue_barrier != NULL)
    {
        // retira tarefa da fila da barreira e adiciona a fila de prontas
        task_t* task = b->queue_barrier;
        queue_remove((queue_t**)&b->queue_barrier, (queue_t*)task);
        queue_append((queue_t**)&__queue_ready_tasks, (queue_t*)task);
    }
    b->N = 0;
    b->n_threads = 0;
    b->state = BARR_DESTROYED;
    
    __not_preempt = aux_preempt;
    return 0;
}


int mqueue_create (mqueue_t *queue, int max, int size)
{
    if(queue == NULL)
        return -1;

    int aux_preempt = __not_preempt;
    // operacoes de filas nao podem ser preemptada
    __not_preempt = 1;

    queue->queue_msg = (void**)malloc(sizeof(void*)*max);
    int i;
    for(i = 0; i < max; i++)
        queue->queue_msg[i] = (void*)malloc(size);
    sem_create(&queue->sem_rcv, 0);
    sem_create(&queue->sem_send, max);
    queue->max_n_msg = max;
    queue->max_size_msg = size;
    queue->n_msg = 0;
    queue->state = MQUEUE_INITIALIZED;
    
    __not_preempt = aux_preempt;
    return 0;
}


int mqueue_send (mqueue_t *queue, void *msg)
{
    if(queue == NULL)
        return -1;
    if(msg == NULL)
        return -1;
    int aux_preempt = __not_preempt;
    // operacoes de filas nao podem ser preemptada
    __not_preempt = 1;
    
    if(sem_down(&queue->sem_send) < 0)
        return -1;
    
    // copia conteudo da mensagem para a fila
    bcopy(msg, queue->queue_msg[queue->n_msg], queue->max_size_msg);
    queue->n_msg++;

    if(sem_up(&queue->sem_rcv))
        return -1;
    
    __not_preempt = aux_preempt;
    return 0;
}


int mqueue_recv (mqueue_t *queue, void *msg)
{
    if(queue == NULL)
        return -1;
    if(msg == NULL)
        return -1;
    int aux_preempt = __not_preempt;
    // operacoes de filas nao podem ser preemptada
    __not_preempt = 1;
    
    if(sem_down(&queue->sem_rcv) < 0)
        return -1;
    
    // copia conteudo da fila para a mensagem
    queue->n_msg--;
    bcopy(queue->queue_msg[queue->n_msg], msg, queue->max_size_msg);
    
    if(sem_up(&queue->sem_send) < 0)
        return -1;
    
    __not_preempt = aux_preempt;
    return 0;
}


int mqueue_destroy (mqueue_t *queue)
{
    if(queue == NULL)
        return -1;
    int aux_preempt = __not_preempt;
    // operacoes de filas nao podem ser preemptada
    __not_preempt = 1;
    int i;
    for(i = 0; i < queue->max_n_msg; i++)
        free(queue->queue_msg[i]);
    free(queue->queue_msg);
    
    sem_destroy(&queue->sem_rcv);
    sem_destroy(&queue->sem_send);

    queue->max_n_msg = 0;
    queue->max_size_msg = 0;
    queue->n_msg = 0;
    queue->state = MQUEUE_DESTROYED;
    
    __not_preempt = aux_preempt;
    return 0;
}


int mqueue_msgs (mqueue_t *queue)
{
    if(queue == NULL)
        return -1;
    if(queue->state == MQUEUE_DESTROYED)
        return -1;
    return queue->n_msg;
}
