#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <unistd.h>

// Tamanho da tela (mantido para compatibilidade)
#define TELA_LARGURA 80
#define TELA_ALTURA 24
#define MAX_OBSTACULOS 10

// Mutexes globais
extern pthread_mutex_t mutex_ponte;
extern pthread_mutex_t mutex_deposito;
extern pthread_mutex_t mutex_tela;

// Enum para níveis de dificuldade
typedef enum {
    FACIL = 1,
    MEDIO = 2,
    DIFICIL = 3
} Dificuldade;

// Configurações globais de dificuldade
extern int capacidade_bateria;
extern useconds_t tempo_recarga_bateria;
extern useconds_t tempo_disparo_min;
extern useconds_t tempo_disparo_max;
extern Dificuldade nivel_dificuldade;

// Função para configurar dificuldade
void configurar_dificuldade(Dificuldade dificuldade);

// Variáveis globais para soldados
extern int soldados_embarcados;
extern int soldados_resgatados;
extern int soldados_total;

// Variável global para controlar o jogo
extern int jogo_rodando;

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