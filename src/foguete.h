#ifndef FOGUETE_H
#define FOGUETE_H

#include <pthread.h>

#define MAX_FOGUETES 10

// Estrutura do foguete
typedef struct {
    int x, y;
    int ativo;
    pthread_mutex_t mutex;
} Foguete;

// Variável global dos foguetes
extern Foguete foguetes[10];

void* thread_func_foguetes(void* arg);
void limpar_foguetes(void);

#endif // FOGUETE_H 