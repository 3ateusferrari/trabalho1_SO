#ifndef BATERIA_H
#define BATERIA_H

#include <pthread.h>

// Estrutura da bateria
typedef struct {
    int id;
    int municao;
    int recarregando;
    int ativa;
    int x, y;
    int nivel;
    pthread_mutex_t mutex;
} Bateria;

// Variáveis globais das baterias
extern Bateria bateria0, bateria1;

void* thread_func_bateria0(void* arg);
void* thread_func_bateria1(void* arg);
void reinicializar_baterias(void);

#endif // BATERIA_H 