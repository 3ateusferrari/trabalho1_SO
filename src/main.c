#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "helicoptero.h"
#include "bateria.h"
#include "foguete.h"
#include "motor.h"
#include "utils.h"
#include <termios.h>

// Threads
pthread_t thread_helicoptero;
pthread_t thread_bateria0;
pthread_t thread_bateria1;
pthread_t thread_foguetes;
pthread_t thread_motor;

// Variável global para controlar o jogo
int jogo_rodando = 1;

void set_input_mode(struct termios* saved_attributes) {
    struct termios tattr;
    tcgetattr(STDIN_FILENO, saved_attributes);
    tcgetattr(STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON | ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

void reset_input_mode(struct termios* saved_attributes) {
    tcsetattr(STDIN_FILENO, TCSANOW, saved_attributes);
}

void* capturar_entrada(void* arg) {
    char entrada;
    while (jogo_rodando) {
        entrada = getchar();
        if (entrada == '\033') { // Código de escape
            getchar(); // '['
            switch(getchar()) {
                case 'A': // Seta para cima
                    pthread_mutex_lock(&helicoptero.mutex);
                    if (helicoptero.y > 0) helicoptero.y--;
                    pthread_mutex_unlock(&helicoptero.mutex);
                    break;
                case 'B': // Seta para baixo
                    pthread_mutex_lock(&helicoptero.mutex);
                    if (helicoptero.y < TELA_ALTURA - 1) helicoptero.y++;
                    pthread_mutex_unlock(&helicoptero.mutex);
                    break;
                case 'C': // Seta para direita
                    pthread_mutex_lock(&helicoptero.mutex);
                    if (helicoptero.x < TELA_LARGURA - 1) helicoptero.x++;
                    pthread_mutex_unlock(&helicoptero.mutex);
                    break;
                case 'D': // Seta para esquerda
                    pthread_mutex_lock(&helicoptero.mutex);
                    if (helicoptero.x > 0) helicoptero.x--;
                    pthread_mutex_unlock(&helicoptero.mutex);
                    break;
            }
        } else if (entrada == 'q' || entrada == 'Q') {
            pthread_mutex_lock(&helicoptero.mutex);
            jogo_rodando = 0;
            helicoptero.vivo = 0;
            pthread_mutex_unlock(&helicoptero.mutex);
        }
    }
    return NULL;
}

int main() {
    struct termios saved_attributes;
    set_input_mode(&saved_attributes);
    srand(time(NULL));
    
    // Seleção de dificuldade
    printf("Selecione o nível de dificuldade:\n");
    printf("1 - Fácil\n2 - Médio\n3 - Difícil\n> ");
    int escolha = 2;
    scanf("%d", &escolha);
    getchar(); // Limpa o enter do buffer
    switch (escolha) {
        case 1:
            nivel_dificuldade = FACIL;
            capacidade_bateria = 3;
            tempo_recarga_bateria = 2000000; // 2 segundos
            break;
        case 2:
            nivel_dificuldade = MEDIO;
            capacidade_bateria = 5;
            tempo_recarga_bateria = 1000000; // 1 segundo
            break;
        case 3:
            nivel_dificuldade = DIFICIL;
            capacidade_bateria = 10;
            tempo_recarga_bateria = 500000; // 0.5 segundo
            break;
        default:
            nivel_dificuldade = MEDIO;
            capacidade_bateria = 5;
            tempo_recarga_bateria = 1000000;
            break;
    }

    // Criação das threads
    pthread_create(&thread_helicoptero, NULL, thread_func_helicoptero, NULL);
    pthread_create(&thread_bateria0, NULL, thread_func_bateria0, NULL);
    pthread_create(&thread_bateria1, NULL, thread_func_bateria1, NULL);
    pthread_create(&thread_foguetes, NULL, thread_func_foguetes, NULL);
    pthread_create(&thread_motor, NULL, thread_func_motor, NULL);

    pthread_t thread_input;
    pthread_create(&thread_input, NULL, capturar_entrada, NULL);

    // Espera as threads terminarem
    pthread_join(thread_helicoptero, NULL);
    pthread_join(thread_bateria0, NULL);
    pthread_join(thread_bateria1, NULL);
    pthread_join(thread_foguetes, NULL);
    pthread_join(thread_motor, NULL);
    pthread_join(thread_input, NULL);

    reset_input_mode(&saved_attributes);

    return 0;
} 