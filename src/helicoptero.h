#ifndef HELICOPTERO_H
#define HELICOPTERO_H

#include <pthread.h>

// Estrutura do helicóptero
typedef struct {
    int x, y; // Posição
    int vivo;
    int movendo_cima, movendo_baixo, movendo_esquerda, movendo_direita;
    pthread_mutex_t mutex;
} Helicoptero;

// Variável global do helicóptero
extern Helicoptero helicoptero;

// Controle de soldados
extern int soldados_embarcados;
extern int soldados_resgatados;
extern int soldados_total;
void resetar_helicoptero();

void* thread_func_helicoptero(void* arg);

#endif // HELICOPTERO_H 