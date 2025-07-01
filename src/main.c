#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "utils.h"
#include "helicoptero.h"
#include "bateria.h"
#include "foguete.h"
#include "motor.h"
#include "graphics.h"

// Variável global para controlar o jogo
extern int jogo_rodando;

int main() {
    srand(time(NULL));
    
    // Inicializar variáveis globais
    soldados_embarcados = 1;
    soldados_resgatados = 0;
    soldados_total = 10;
    jogo_rodando = 1;
    
    // Inicializar sistema gráfico
    if (!graphics_init()) {
        printf("Erro ao inicializar sistema gráfico!\n");
        return 1;
    }
    
    printf("Jogo iniciado! Use a interface gráfica para jogar.\n");
    printf("Pressione ESC para sair\n");

    // Variáveis para as threads (serão criadas quando o jogo começar)
    pthread_t thread_helicoptero = 0, thread_bateria = 0, thread_foguete = 0, thread_motor = 0, thread_bateria1 = 0;
    int threads_criadas = 0;

    // Loop principal do jogo (interface gráfica)
    while (jogo_rodando) {
        // Processar eventos primeiro
        graphics_handle_events();
        
        // Desenhar baseado no estado atual
        GameState current_state = graphics_get_state();
        
        switch (current_state) {
            case MENU_PRINCIPAL:
                graphics_draw_main_menu();
                break;
                
            case DIFICULDADE:
                graphics_draw_difficulty_menu();
                break;
                
            case JOGO:
                // Criar threads apenas na primeira vez que entrar no jogo
                if (!threads_criadas) {
                    // Aplicar dificuldade selecionada
                    configurar_dificuldade(graphics_get_selected_difficulty());
                    printf("Dificuldade configurada: %d\n", nivel_dificuldade);
                    
                    // Resetar estado do jogo
                    soldados_embarcados = 1;
                    soldados_resgatados = 0;
                    jogo_rodando = 1;
                    
                    // Criar threads do jogo
                    if (pthread_create(&thread_helicoptero, NULL, thread_func_helicoptero, NULL) != 0) {
                        printf("Erro ao criar thread do helicóptero\n");
                        return 1;
                    }
                    
                    if (pthread_create(&thread_bateria, NULL, thread_func_bateria0, NULL) != 0) {
                        printf("Erro ao criar thread da bateria 0\n");
                        return 1;
                    }
                    
                    if (pthread_create(&thread_bateria1, NULL, thread_func_bateria1, NULL) != 0) {
                        printf("Erro ao criar thread da bateria 1\n");
                        return 1;
                    }
                    
                    if (pthread_create(&thread_foguete, NULL, thread_func_foguetes, NULL) != 0) {
                        printf("Erro ao criar thread do foguete\n");
                        return 1;
                    }
                    
                    if (pthread_create(&thread_motor, NULL, thread_func_motor, NULL) != 0) {
                        printf("Erro ao criar thread do motor\n");
                        return 1;
                    }
                    
                    threads_criadas = 1;
                    printf("Threads do jogo criadas!\n");
                    
                    // Aguardar um pouco para as threads inicializarem e depois reinicializar baterias
                    SDL_Delay(100);
                    limpar_foguetes();
                    reinicializar_baterias();
                }
                graphics_draw_game_screen();
                break;
                
            case GAME_OVER:
                graphics_draw_game_over();
                break;
                
            case VITORIA:
                graphics_draw_victory();
                break;
        }
        
        // Verificar condições de fim de jogo
        if (!helicoptero.vivo && current_state == JOGO) {
            graphics_set_state(GAME_OVER);
        } else if (soldados_resgatados >= soldados_total && current_state == JOGO) {
            graphics_set_state(VITORIA);
        }
        
        // Resetar threads quando voltar ao menu principal
        if (current_state == MENU_PRINCIPAL && threads_criadas) {
            threads_criadas = 0;
            printf("Jogo resetado - threads serão recriadas na próxima partida\n");
        }
        
        // Pequeno delay para controlar FPS
        SDL_Delay(50); // ~20 FPS para debug
    }
    
    // Aguardar threads terminarem (apenas se foram criadas)
    if (threads_criadas) {
        pthread_join(thread_helicoptero, NULL);
        pthread_join(thread_bateria, NULL);
        pthread_join(thread_bateria1, NULL);
        pthread_join(thread_foguete, NULL);
        pthread_join(thread_motor, NULL);
        printf("Todas as threads finalizadas!\n");
    }
    
    // Limpar sistema gráfico
    graphics_cleanup();
    printf("Jogo finalizado!\n");

    return 0;
} 