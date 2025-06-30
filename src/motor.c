#include "motor.h"
#include "helicoptero.h"
#include "bateria.h"
#include "foguete.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

extern int jogo_rodando;

// Função para limpar a tela (simples)
void limpar_tela() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

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

// Função para desenhar a matriz da tela
void desenhar_tela() {
    // Limpa matriz
    for (int y = 0; y < TELA_ALTURA; y++) {
        memset(tela[y], ' ', TELA_LARGURA);
        tela[y][TELA_LARGURA] = '\0';
    }
    // Desenha plataforma
    for (int y = 0; y < plataforma.altura; y++) {
        for (int x = 0; x < plataforma.largura; x++) {
            int px = plataforma.x + x;
            int py = plataforma.y + y;
            if (px < TELA_LARGURA && py < TELA_ALTURA)
                tela[py][px] = '=';
        }
    }
    // Desenha obstáculos
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo && obstaculos[i].x < TELA_LARGURA && obstaculos[i].y < TELA_ALTURA)
            tela[obstaculos[i].y][obstaculos[i].x] = '#';
    }
    // Desenha foguetes
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        if (foguetes[i].ativo && foguetes[i].x >= 0 && foguetes[i].x < TELA_LARGURA && foguetes[i].y >= 0 && foguetes[i].y < TELA_ALTURA)
            tela[foguetes[i].y][foguetes[i].x] = '*';
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    // Desenha helicóptero
    pthread_mutex_lock(&helicoptero.mutex);
    if (helicoptero.vivo && helicoptero.x >= 0 && helicoptero.x < TELA_LARGURA && helicoptero.y >= 0 && helicoptero.y < TELA_ALTURA)
        tela[helicoptero.y][helicoptero.x] = 'H';
    pthread_mutex_unlock(&helicoptero.mutex);
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
    pthread_mutex_lock(&helicoptero.mutex);
    int hx = helicoptero.x;
    int hy = helicoptero.y;
    pthread_mutex_unlock(&helicoptero.mutex);
    if (hx >= plataforma.x && hx < plataforma.x + plataforma.largura &&
        hy >= plataforma.y && hy < plataforma.y + plataforma.altura)
        return 1;
    return 0;
}

// Função para verificar colisão com baterias, depósito ou plataforma (exceto pouso)
int colisao_com_estruturas(int x, int y) {
    // Colisão com baterias (posição fixa à esquerda)
    if ((x == 0 && (y == 5 || y == 10)) || // Exemplo: baterias em y=5 e y=10
        (x == 5 && y == TELA_ALTURA-2)) // Exemplo: depósito
        return 1;
    // Colisão com plataforma (exceto pouso)
    if (x >= plataforma.x && x < plataforma.x + plataforma.largura &&
        y >= plataforma.y && y < plataforma.y + plataforma.altura)
        return 2; // Plataforma
    return 0;
}

void renderizar_jogo() {
    desenhar_tela();
    limpar_tela();
    printf("=== JOGO DO HELICÓPTERO ===\n");
    printf("Soldados resgatados: %d/%d\n", soldados_resgatados, soldados_total);
    printf("Controles: setas (mover), q (sair)\n\n");
    for (int y = 0; y < TELA_ALTURA; y++) {
        printf("%s\n", tela[y]);
    }
    // Status
    pthread_mutex_lock(&helicoptero.mutex);
    printf("Helicóptero: (%d, %d) - %s - Soldados a bordo: %d\n", helicoptero.x, helicoptero.y, helicoptero.vivo ? "VIVO" : "MORTO", soldados_embarcados);
    pthread_mutex_unlock(&helicoptero.mutex);
    pthread_mutex_lock(&bateria0.mutex);
    printf("Bateria 0: %d munições - %s\n", bateria0.municao, bateria0.recarregando ? "RECARREGANDO" : "PRONTA");
    pthread_mutex_unlock(&bateria0.mutex);
    pthread_mutex_lock(&bateria1.mutex);
    printf("Bateria 1: %d munições - %s\n", bateria1.municao, bateria1.recarregando ? "RECARREGANDO" : "PRONTA");
    pthread_mutex_unlock(&bateria1.mutex);
    // Foguetes ativos
    printf("Foguetes ativos: ");
    int foguetes_ativos = 0;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        if (foguetes[i].ativo) {
            printf("%d(%d,%d) ", i, foguetes[i].x, foguetes[i].y);
            foguetes_ativos++;
        }
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    if (foguetes_ativos == 0) printf("Nenhum");
    printf("\n\n");
    // Fim de jogo
    pthread_mutex_lock(&helicoptero.mutex);
    if (!helicoptero.vivo) {
        jogo_rodando = 0;
        printf("=== FIM DE JOGO ===\nHelicóptero foi destruído!\n");
    }
    pthread_mutex_unlock(&helicoptero.mutex);
    if (soldados_resgatados >= soldados_total) {
        jogo_rodando = 0;
        printf("=== VITÓRIA! ===\nTodos os soldados foram resgatados!\n");
    }
}

void* thread_func_motor(void* arg) {
    inicializar_obstaculos_e_plataforma();
    resetar_helicoptero();
    while (jogo_rodando) {
        pthread_mutex_lock(&helicoptero.mutex);
        // Colisão com topo
        if (helicoptero.y == 0) {
            helicoptero.vivo = 0;
        }
        // Colisão com solo
        if (helicoptero.y == TELA_ALTURA-1) {
            helicoptero.vivo = 0;
        }
        // Colisão com obstáculos
        if (colisao_com_obstaculo(helicoptero.x, helicoptero.y)) {
            helicoptero.vivo = 0;
        }
        // Colisão com baterias, depósito, plataforma (exceto pouso)
        int col = colisao_com_estruturas(helicoptero.x, helicoptero.y);
        if (col == 1) {
            helicoptero.vivo = 0;
        }
        // Desembarque na plataforma
        if (col == 2 && helicoptero.vivo && soldados_embarcados > 0) {
            soldados_resgatados += soldados_embarcados;
            if (soldados_resgatados < soldados_total) {
                resetar_helicoptero();
            }
        }
        pthread_mutex_unlock(&helicoptero.mutex);
        renderizar_jogo();
        usleep(150000);
    }
    return NULL;
} 