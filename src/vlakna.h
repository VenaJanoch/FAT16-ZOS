#ifndef VLAKNA_H
#define VLAKNA_H
#include <pthread.h>
#include "fat.h"

/* Struktura pro worker */
typedef struct {
	unsigned int first_cluster;         /* Prvni cluster */
    unsigned int last_cluster;          /* Posledni cluster */
    pthread_t thread;                   /* Vlakno */
    pthread_mutex_t *allocation_mutex;  /* Mutex */
} worker_struct;

/* Opraveni badblocku - worker */
void *oprav_bloky_worker(void *data_);

/* Opraveni badblocku - farmer */
int oprav_blok(char* fat);


#endif
