// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// interface do driver de disco rígido

#ifndef __DISKDRIVER__
#define __DISKDRIVER__
#include "datatypes.h"
#include "harddisk.h"

// defines relacionados ao disco
#define NUM_BLOCKS 256
#define BLOCK_SIZE 64
// tipos de requisicao ao disco
#define DISK_READ 0
#define DISK_WRITE 1
// estados do disco
#define DISK_BUSY 0
#define DISK_FREE 1


// structura de dados que representa o disco para o SO
typedef struct
{
    // Numero de blocos percorridos pela cabeca do disco
    unsigned int total_blocks;
    // Tempo total de uso do disco
    unsigned int total_usage_time;
    // Tempo da ultima ativacao
    unsigned int last_activation;
    // Bloco da ultima operacao do disco
    int last_disk_block;
    // semaforo do disco
    semaphore_t sem_disk;
    
} disk_t ;

// inicializacao do driver de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int diskdriver_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer indicado
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer indicado para o disco
int disk_block_write (int block, void *buffer) ;

#endif
