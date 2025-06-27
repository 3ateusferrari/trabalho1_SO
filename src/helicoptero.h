#ifndef HELICOPTERO_H
#define HELICOPTERO_H

#include <pthread.h>

// Estrutura do helicóptero
typedef struct {
    int x, y; // Posição
    int vivo;
    pthread_mutex_t mutex;
} Helicoptero;

// Variável global do helicóptero
extern Helicoptero helicoptero;

void* thread_func_helicoptero(void* arg);

#endif // HELICOPTERO_H 