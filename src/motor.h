#ifndef MOTOR_H
#define MOTOR_H

#include <pthread.h>

// Estrutura do motor
typedef struct {
    int bateria_atual;
    pthread_mutex_t mutex;
} Motor;

// Variável global do motor
extern Motor motor;

void* thread_func_motor(void* arg);

#endif // MOTOR_H 