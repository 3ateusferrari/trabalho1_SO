#include "foguete.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>

// Array de foguetes
Foguete foguetes[10];

void* thread_func_foguetes(void* arg) {
    // Inicializa foguetes
    for (int i = 0; i < 10; i++) {
        foguetes[i].ativo = 0;
        foguetes[i].x = 0;
        foguetes[i].y = 0;
        pthread_mutex_init(&foguetes[i].mutex, NULL);
    }
    
    while (jogo_rodando) {
        for (int i = 0; i < 10; i++) {
            pthread_mutex_lock(&foguetes[i].mutex);
            
            if (foguetes[i].ativo) {
                // Move foguete para a esquerda
                foguetes[i].x--;
                
                // Se foguete saiu da tela, desativa
                if (foguetes[i].x < 0) {
                    foguetes[i].ativo = 0;
                }
            }
            
            pthread_mutex_unlock(&foguetes[i].mutex);
        }
        
        usleep(100000); // Atualiza a cada 100ms
    }
    return NULL;
}

// Função para limpar todos os foguetes
void limpar_foguetes(void) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        foguetes[i].ativo = 0;
        foguetes[i].x = 0;
        foguetes[i].y = 0;
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    printf("Todos os foguetes foram limpos\n");
} 