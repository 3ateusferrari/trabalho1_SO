#include "helicoptero.h"
#include "utils.h"
#include "foguete.h"
#include <stdio.h>
#include <unistd.h>

Helicoptero helicoptero;
int soldados_embarcados = 0;
int soldados_resgatados = 0;
int soldados_total = 10;

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
    soldados_embarcados = 1;
}

void* thread_func_helicoptero(void* arg) {
    // Inicializa posição do helicóptero
    helicoptero.x = 2;
    helicoptero.y = TELA_ALTURA / 2;
    helicoptero.vivo = 1;
    pthread_mutex_init(&helicoptero.mutex, NULL);

    while (helicoptero.vivo) {
        pthread_mutex_lock(&helicoptero.mutex);
        
        // Verifica colisões
        if (verificar_colisao_foguetes()) {
            helicoptero.vivo = 0;
        }
        
        // Verifica colisão com bordas da tela
        if (helicoptero.y < 0) helicoptero.y = 0;
        if (helicoptero.y >= TELA_ALTURA) helicoptero.y = TELA_ALTURA - 1;
        if (helicoptero.x < 0) helicoptero.x = 0;
        if (helicoptero.x >= TELA_LARGURA) helicoptero.x = TELA_LARGURA - 1;
        
        pthread_mutex_unlock(&helicoptero.mutex);
        
        usleep(50000); // 50ms de atualização
    }
    
    return NULL;
} 