#include "utils.h"

pthread_mutex_t mutex_ponte = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deposito = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;

char tela[TELA_ALTURA][TELA_LARGURA+1];
Obstaculo obstaculos[MAX_OBSTACULOS];
Plataforma plataforma;

Dificuldade nivel_dificuldade = MEDIO;
int capacidade_bateria = 5;
useconds_t tempo_recarga_bateria = 1000000; 