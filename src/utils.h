#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>

// Tamanho da tela
#define TELA_LARGURA 80
#define TELA_ALTURA 24
#define MAX_OBSTACULOS 10

// Mutexes globais
extern pthread_mutex_t mutex_ponte;
extern pthread_mutex_t mutex_deposito;
extern pthread_mutex_t mutex_tela;

typedef struct {
    int x, y;
    int ativo;
} Obstaculo;

typedef struct {
    int x, y;
    int largura, altura;
} Plataforma;

extern char tela[TELA_ALTURA][TELA_LARGURA+1];
extern Obstaculo obstaculos[MAX_OBSTACULOS];
extern Plataforma plataforma;

#endif