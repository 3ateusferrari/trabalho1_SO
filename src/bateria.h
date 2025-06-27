#ifndef BATERIA_H
#define BATERIA_H

#include <pthread.h>

// Estrutura da bateria
typedef struct {
    int id;
    int municao;
    int recarregando;
    pthread_mutex_t mutex;
} Bateria;

// Variáveis globais das baterias
extern Bateria bateria0, bateria1;

void* thread_func_bateria0(void* arg);
void* thread_func_bateria1(void* arg);

#endif // BATERIA_H 