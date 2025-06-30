#include "bateria.h"
#include "utils.h"
#include "foguete.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

Bateria bateria0, bateria1;

// Função para encontrar um foguete disponível
int encontrar_foguete_livre() {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        if (!foguetes[i].ativo) {
            foguetes[i].ativo = 1;
            pthread_mutex_unlock(&foguetes[i].mutex);
            return i;
        }
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    return -1; // Nenhum foguete disponível
}

// Função para recarregar bateria
void recarregar_bateria(Bateria* bat) {
    // Travessia da ponte (ida)
    pthread_mutex_lock(&mutex_ponte);
    usleep(200000); // Simula travessia
    pthread_mutex_unlock(&mutex_ponte);
    
    // Acesso ao depósito
    pthread_mutex_lock(&mutex_deposito);
    usleep(500000); // Simula tempo de recarga
    bat->municao = 5;
    bat->recarregando = 0;
    pthread_mutex_unlock(&mutex_deposito);
    
    // Travessia da ponte (volta)
    pthread_mutex_lock(&mutex_ponte);
    usleep(200000); // Simula travessia
    pthread_mutex_unlock(&mutex_ponte);
}

void* thread_func_bateria0(void* arg) {
    bateria0.id = 0;
    bateria0.municao = 5;
    bateria0.recarregando = 0;
    pthread_mutex_init(&bateria0.mutex, NULL);
    
    while (1) {
        pthread_mutex_lock(&bateria0.mutex);
        
        if (bateria0.municao > 0 && !bateria0.recarregando) {
            // Dispara foguete
            int foguete_id = encontrar_foguete_livre();
            if (foguete_id != -1) {
                foguetes[foguete_id].x = TELA_LARGURA - 10; // Posição da bateria
                foguetes[foguete_id].y = rand() % TELA_ALTURA; // Direção aleatória
                bateria0.municao--;
            }
        } else if (bateria0.municao == 0 && !bateria0.recarregando) {
            // Inicia recarga
            bateria0.recarregando = 1;
            pthread_mutex_unlock(&bateria0.mutex);
            recarregar_bateria(&bateria0);
            continue;
        }
        
        pthread_mutex_unlock(&bateria0.mutex);
        usleep(300000 + (rand() % 200000)); // Disparo a cada 3-5 segundos
    }
    return NULL;
}

void* thread_func_bateria1(void* arg) {
    bateria1.id = 1;
    bateria1.municao = 5;
    bateria1.recarregando = 0;
    pthread_mutex_init(&bateria1.mutex, NULL);
    
    while (1) {
        pthread_mutex_lock(&bateria1.mutex);
        
        if (bateria1.municao > 0 && !bateria1.recarregando) {
            // Dispara foguete
            int foguete_id = encontrar_foguete_livre();
            if (foguete_id != -1) {
                foguetes[foguete_id].x = TELA_LARGURA - 15; // Posição diferente da bateria 0
                foguetes[foguete_id].y = rand() % TELA_ALTURA; // Direção aleatória
                bateria1.municao--;
            }
        } else if (bateria1.municao == 0 && !bateria1.recarregando) {
            // Inicia recarga
            bateria1.recarregando = 1;
            pthread_mutex_unlock(&bateria1.mutex);
            recarregar_bateria(&bateria1);
            continue;
        }
        
        pthread_mutex_unlock(&bateria1.mutex);
        usleep(400000 + (rand() % 300000)); // Disparo a cada 4-7 segundos (diferente da bateria 0)
    }
    return NULL;
} 