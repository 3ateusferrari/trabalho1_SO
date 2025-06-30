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
        
        pthread_mutex_lock(&helicoptero.mutex);
        switch (entrada) {
            case 'w':
            case 'W':
                if (helicoptero.y > 0) helicoptero.y--;
                break;
            case 's':
            case 'S':
                if (helicoptero.y < TELA_ALTURA - 1) helicoptero.y++;
                break;
            case 'a':
            case 'A':
                if (helicoptero.x > 0) helicoptero.x--;
                break;
            case 'd':
            case 'D':
                if (helicoptero.x < TELA_LARGURA - 1) helicoptero.x++;
                break;
            case 'q':
            case 'Q':
                jogo_rodando = 0;
                helicoptero.vivo = 0;
                break;
        }
        pthread_mutex_unlock(&helicoptero.mutex);
    }
    return NULL;
}

int main() {
    struct termios saved_attributes;
    set_input_mode(&saved_attributes);
    srand(time(NULL));
    
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