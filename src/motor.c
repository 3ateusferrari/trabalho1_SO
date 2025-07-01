#include "motor.h"
#include "helicoptero.h"
#include "bateria.h"
#include "foguete.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

extern int jogo_rodando;

// Variável global do motor
Motor motor;

// Função para inicializar obstáculos e plataforma
void inicializar_obstaculos_e_plataforma() {
    // Obstáculos fixos
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        obstaculos[i].x = 20 + i * 5;
        obstaculos[i].y = 5 + (i % 10);
        obstaculos[i].ativo = 1;
    }
    // Plataforma no canto inferior direito
    plataforma.x = TELA_LARGURA - 8;
    plataforma.y = TELA_ALTURA - 3;
    plataforma.largura = 6;
    plataforma.altura = 2;
}

// Função para verificar colisão com obstáculos
int colisao_com_obstaculo(int x, int y) {
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo && obstaculos[i].x == x && obstaculos[i].y == y)
            return 1;
    }
    return 0;
}

// Função para verificar se helicóptero está na plataforma
int helicoptero_na_plataforma() {
    int hx, hy;
    pthread_mutex_lock(&helicoptero.mutex);
    hx = helicoptero.x;
    hy = helicoptero.y;
    pthread_mutex_unlock(&helicoptero.mutex);
    
    if (hx >= plataforma.x && hx < plataforma.x + plataforma.largura &&
        hy >= plataforma.y && hy < plataforma.y + plataforma.altura)
        return 1;
    return 0;
}

// Função para verificar colisão com baterias, depósito ou plataforma (exceto pouso)
int colisao_com_estruturas(int x, int y) {
    // Colisão com baterias (posição à direita)
    if ((x == TELA_LARGURA - 5 && y == 5) || // Bateria 0
        (x == TELA_LARGURA - 10 && y == 10)) // Bateria 1
        return 1;
    
    // Colisão com depósito (esquerda)
    if (x <= 5 && y >= TELA_ALTURA - 3)
        return 1;
    
    // Colisão com plataforma (exceto pouso)
    if (x >= plataforma.x && x < plataforma.x + plataforma.largura &&
        y >= plataforma.y && y < plataforma.y + plataforma.altura)
        return 2; // Plataforma
    return 0;
}

void* thread_func_motor(void* arg) {
    // Inicializar motor
    motor.bateria_atual = 100;
    pthread_mutex_init(&motor.mutex, NULL);
    
    inicializar_obstaculos_e_plataforma();
    resetar_helicoptero();
    
    while (jogo_rodando) {
        pthread_mutex_lock(&helicoptero.mutex);
        
        // Verificar se helicóptero ainda está vivo
        if (!helicoptero.vivo) {
            pthread_mutex_unlock(&helicoptero.mutex);
            jogo_rodando = 0;
            printf("Game Over! Helicóptero destruído!\n");
            break;
        }
        
        // Colisão com solo
        if (helicoptero.y >= TELA_ALTURA - 1) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com o solo!\n");
        }
        
        // Colisão com obstáculos
        if (colisao_com_obstaculo(helicoptero.x, helicoptero.y)) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com obstáculo!\n");
        }
        
        // Colisão com baterias, depósito, plataforma (exceto pouso)
        int col = colisao_com_estruturas(helicoptero.x, helicoptero.y);
        if (col == 1) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com estrutura!\n");
        }
        
        // Desembarque na plataforma
        if (col == 2 && helicoptero.vivo && soldados_embarcados > 0) {
            soldados_resgatados += soldados_embarcados;
            printf("Soldados resgatados! Total: %d/%d\n", soldados_resgatados, soldados_total);
            
            if (soldados_resgatados >= soldados_total) {
                // Vitória!
                jogo_rodando = 0;
                printf("VITÓRIA! Todos os soldados foram resgatados!\n");
                pthread_mutex_unlock(&helicoptero.mutex);
                break;
            } else {
                // Resetar helicóptero para buscar mais soldados
                resetar_helicoptero();
            }
        }
        
        pthread_mutex_unlock(&helicoptero.mutex);

        usleep(150000);
    }
    return NULL;
} 