#include "helicoptero.h"
#include "utils.h"
#include "foguete.h"
#include <stdio.h>
#include <unistd.h>

Helicoptero helicoptero;
// int soldados_embarcados = 0;
// int soldados_resgatados = 0;
// int soldados_total = 10;

// Função para verificar colisão com foguetes
int verificar_colisao_foguetes() {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        if (foguetes[i].ativo && 
            foguetes[i].x == helicoptero.x && 
            foguetes[i].y == helicoptero.y) {
            pthread_mutex_unlock(&foguetes[i].mutex);
            return 1; // Colisão detectada
        }
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    return 0; // Sem colisão
}

void resetar_helicoptero() {
    helicoptero.x = 2;
    helicoptero.y = TELA_ALTURA / 2;
    helicoptero.vivo = 1;
    helicoptero.movendo_cima = 0;
    helicoptero.movendo_baixo = 0;
    helicoptero.movendo_esquerda = 0;
    helicoptero.movendo_direita = 0;
    soldados_embarcados = 1;
}

void* thread_func_helicoptero(void* arg) {
    // Inicializa posição do helicóptero
    helicoptero.x = 2;
    helicoptero.y = TELA_ALTURA / 2;
    helicoptero.vivo = 1;
    helicoptero.movendo_cima = 0;
    helicoptero.movendo_baixo = 0;
    helicoptero.movendo_esquerda = 0;
    helicoptero.movendo_direita = 0;
    pthread_mutex_init(&helicoptero.mutex, NULL);

    while (helicoptero.vivo && jogo_rodando) {
        pthread_mutex_lock(&helicoptero.mutex);
        
        // Aplicar movimento baseado nas teclas pressionadas
        if (helicoptero.movendo_cima && helicoptero.y > 0) {
            helicoptero.y--;
        }
        if (helicoptero.movendo_baixo && helicoptero.y < TELA_ALTURA - 1) {
            helicoptero.y++;
        }
        if (helicoptero.movendo_esquerda && helicoptero.x > 0) {
            helicoptero.x--;
        }
        if (helicoptero.movendo_direita && helicoptero.x < TELA_LARGURA - 1) {
            helicoptero.x++;
        }
        
        // Verifica colisões
        if (verificar_colisao_foguetes()) {
            helicoptero.vivo = 0;
            printf("Helicóptero atingido por foguete!\n");
        }
        
        // Verifica colisão com topo da tela (explosão)
        if (helicoptero.y <= 0) {
            helicoptero.vivo = 0;
            printf("Helicóptero explodiu ao atingir o topo!\n");
        }
        
        pthread_mutex_unlock(&helicoptero.mutex);
        
        usleep(50000); // 50ms de atualização
    }
    
    return NULL;
} 