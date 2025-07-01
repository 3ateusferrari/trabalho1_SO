#include "utils.h"
#include <stdio.h>

pthread_mutex_t mutex_ponte = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deposito = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;

char tela[TELA_ALTURA][TELA_LARGURA+1];
Obstaculo obstaculos[MAX_OBSTACULOS];
Plataforma plataforma;

Dificuldade nivel_dificuldade = MEDIO;
int capacidade_bateria = 5;
useconds_t tempo_recarga_bateria = 1000000;
useconds_t tempo_disparo_min = 300000;
useconds_t tempo_disparo_max = 500000;

// Variáveis globais para soldados
int soldados_embarcados = 1;
int soldados_resgatados = 0;
int soldados_total = 10;

// Variável global para controlar o jogo
int jogo_rodando = 1;

// Função para configurar dificuldade
void configurar_dificuldade(Dificuldade dificuldade) {
    nivel_dificuldade = dificuldade;
    
    switch (dificuldade) {
        case FACIL:
            capacidade_bateria = 2;  // Muito poucos foguetes
            tempo_recarga_bateria = 2000000; // 2.0s - recarga muito alta
            tempo_disparo_min = 800000; // 0.8s - disparo lento
            tempo_disparo_max = 1500000; // 1.5s - disparo muito lento
            break;
        case MEDIO:
            capacidade_bateria = 5;  // Quantidade média
            tempo_recarga_bateria = 800000; // 0.8s - recarga média
            tempo_disparo_min = 400000; // 0.4s - disparo médio
            tempo_disparo_max = 800000; // 0.8s - disparo médio-lento
            break;
        case DIFICIL:
            capacidade_bateria = 10; // Muitos foguetes
            tempo_recarga_bateria = 300000; // 0.3s - recarga baixa
            tempo_disparo_min = 200000; // 0.2s - disparo rápido
            tempo_disparo_max = 400000; // 0.4s - disparo médio-rápido
            break;
    }
    
    printf("Dificuldade configurada: %d (capacidade: %d, recarga: %d ms, disparo: %d-%d ms)\n", 
           dificuldade, capacidade_bateria, (int)(tempo_recarga_bateria/1000), 
           (int)(tempo_disparo_min/1000), (int)(tempo_disparo_max/1000));
} 