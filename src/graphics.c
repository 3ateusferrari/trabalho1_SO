#include "graphics.h"
#include "helicoptero.h"
#include "bateria.h"
#include "foguete.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utils.h"

// Variáveis globais necessárias
extern int jogo_rodando;
extern int soldados_embarcados;
extern int soldados_resgatados;
extern int soldados_total;
#define MAX_FOGUETES 10

GraphicsSystem graphics = {0};

int graphics_init(void) {
    printf("Iniciando SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    printf("SDL inicializado com sucesso!\n");
    
    printf("Criando janela...\n");
    graphics.window = SDL_CreateWindow("Helicoptero Game", 
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                      SCREEN_WIDTH, SCREEN_HEIGHT, 
                                      SDL_WINDOW_SHOWN);
    if (!graphics.window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    printf("Janela criada com sucesso!\n");
    
    printf("Criando renderer...\n");
    graphics.renderer = SDL_CreateRenderer(graphics.window, -1, SDL_RENDERER_ACCELERATED);
    if (!graphics.renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    printf("Renderer criado com sucesso!\n");
    
    // Forçar atualização da janela
    SDL_ShowWindow(graphics.window);
    SDL_RaiseWindow(graphics.window);
    
    // Garantir que a janela seja visível
    SDL_SetWindowFullscreen(graphics.window, 0);
    SDL_RestoreWindow(graphics.window);
    
    // Inicializar estado
    graphics.current_state = MENU_PRINCIPAL;
    graphics.selected_difficulty = 2; // Médio por padrão
    graphics.selected_menu_option = 0; // JOGAR selecionado por padrão
    graphics.initialized = 1;
    
    printf("Sistema gráfico inicializado com sucesso!\n");
    return 1;
}

void graphics_cleanup(void) {
    printf("Limpando sistema gráfico...\n");
    if (graphics.renderer) {
        SDL_DestroyRenderer(graphics.renderer);
        graphics.renderer = NULL;
    }
    if (graphics.window) {
        SDL_DestroyWindow(graphics.window);
        graphics.window = NULL;
    }
    SDL_Quit();
    printf("Sistema gráfico limpo!\n");
}





void graphics_draw_ui(int soldados_embarcados, int soldados_resgatados, int soldados_total, int bateria_atual) {
    if (!graphics.renderer) return;
    
    SDL_Rect ui_rect = {0, 0, SCREEN_WIDTH, 40};
    SDL_SetRenderDrawColor(graphics.renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(graphics.renderer, &ui_rect);
}

void graphics_draw_text_centered(const char* text, int y, void* font, SDL_Color color) {
    if (!graphics.renderer) return;
    
    // Desenhar retângulo colorido como placeholder para texto
    SDL_Rect rect = {SCREEN_WIDTH/2 - 100, y, 200, 30};
    SDL_SetRenderDrawColor(graphics.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(graphics.renderer, &rect);
}

void graphics_draw_button(int x, int y, int width, int height, const char* text, int selected) {
    if (!graphics.renderer) return;
    
    // Cor de fundo do botão
    SDL_Rect button_rect = {x, y, width, height};
    if (selected) {
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // Laranja
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 100, 100, 100, 255); // Cinza
    }
    SDL_RenderFillRect(graphics.renderer, &button_rect);
    
    // Borda do botão
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &button_rect);
}

void graphics_draw_main_menu(void) {
    if (!graphics.renderer) {
        return;
    }
    
    // Limpar tela com fundo azul
    SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255);
    SDL_RenderClear(graphics.renderer);
    
    // Forçar atualização da janela
    SDL_UpdateWindowSurface(graphics.window);
    
    // Desenhar título (retângulo branco)
    graphics_draw_text_centered("HELICOPTERO GAME", 100, NULL, (SDL_Color){255, 255, 255, 255});
    
    // Desenhar botões
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 200, 200, 50, "JOGAR", graphics.selected_menu_option == 0);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 270, 200, 50, "DIFICULDADE", graphics.selected_menu_option == 1);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 340, 200, 50, "SAIR", graphics.selected_menu_option == 2);
    
    // Desenhar instruções (retângulo branco)
    graphics_draw_text_centered("Use SETAS para navegar, ENTER para selecionar", 450, NULL, (SDL_Color){255, 255, 255, 255});
    
    // APRESENTAR O FRAME
    SDL_RenderPresent(graphics.renderer);
    
    // Forçar atualização da janela novamente
    SDL_UpdateWindowSurface(graphics.window);
}

void graphics_draw_difficulty_menu(void) {
    if (!graphics.renderer) return;
    
    // Limpar tela com fundo verde
    SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255);
    SDL_RenderClear(graphics.renderer);
    
    // Desenhar título
    graphics_draw_text_centered("SELECIONAR DIFICULDADE", 100, NULL, (SDL_Color){255, 255, 255, 255});
    
    // Desenhar botões de dificuldade
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 200, 200, 50, "FACIL", graphics.selected_difficulty == 1);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 270, 200, 50, "MEDIO", graphics.selected_difficulty == 2);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 340, 200, 50, "DIFICIL", graphics.selected_difficulty == 3);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 410, 200, 50, "VOLTAR", 0);
    
    // Desenhar instruções
    graphics_draw_text_centered("Use SETAS para navegar, ENTER para selecionar", 500, NULL, (SDL_Color){255, 255, 255, 255});
    
    // APRESENTAR O FRAME
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_game_screen(void) {
    if (!graphics.renderer) return;
    
    // Limpar tela com fundo preto
    SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
    SDL_RenderClear(graphics.renderer);
    
    // Desenhar solo (linha inferior)
    SDL_Rect solo_rect = {0, SCREEN_HEIGHT - (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH, SCREEN_HEIGHT / TELA_ALTURA};
    SDL_SetRenderDrawColor(graphics.renderer, 139, 69, 19, 255); // marrom
    SDL_RenderFillRect(graphics.renderer, &solo_rect);
    
    // Desenhar obstáculos
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            int x = (obstaculos[i].x * SCREEN_WIDTH) / TELA_LARGURA;
            int y = (obstaculos[i].y * SCREEN_HEIGHT) / TELA_ALTURA;
            SDL_Rect obs_rect = {x, y, SCREEN_WIDTH / TELA_LARGURA, SCREEN_HEIGHT / TELA_ALTURA};
            SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255); // marrom claro
            SDL_RenderDrawRect(graphics.renderer, &obs_rect);
        }
    }
    // Desenhar depósito (esquerda)
    SDL_Rect deposito_rect = {5, SCREEN_HEIGHT - 3 * (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH / 16, 2 * (SCREEN_HEIGHT / TELA_ALTURA)};
    SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255);
    SDL_RenderDrawRect(graphics.renderer, &deposito_rect);
    // Desenhar ponte (horizontal, entre depósito e solo)
    SDL_Rect ponte_rect = {SCREEN_WIDTH / 16, SCREEN_HEIGHT - (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH / 3, SCREEN_HEIGHT / (2 * TELA_ALTURA)};
    SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255);
    SDL_RenderFillRect(graphics.renderer, &ponte_rect);
    // Desenhar plataforma (direita)
    int plat_x = (plataforma.x * SCREEN_WIDTH) / TELA_LARGURA;
    int plat_y = (plataforma.y * SCREEN_HEIGHT) / TELA_ALTURA;
    SDL_Rect plat_rect = {plat_x, plat_y, (plataforma.largura * SCREEN_WIDTH) / TELA_LARGURA, (plataforma.altura * SCREEN_HEIGHT) / TELA_ALTURA};
    SDL_SetRenderDrawColor(graphics.renderer, 34, 139, 34, 255); // verde escuro
    SDL_RenderFillRect(graphics.renderer, &plat_rect);
    // Borda da plataforma
    SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255); // verde
    SDL_RenderDrawRect(graphics.renderer, &plat_rect);
    // Desenhar helicóptero (verde)
    extern Helicoptero helicoptero;
    if (helicoptero.vivo) {
        int x = (helicoptero.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (helicoptero.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect heli_rect = {x, y, 20, 15};
        SDL_SetRenderDrawColor(graphics.renderer, 173, 255, 47, 255); // verde claro
        SDL_RenderFillRect(graphics.renderer, &heli_rect);
        // Borda do helicóptero
        SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255); // verde
        SDL_RenderDrawRect(graphics.renderer, &heli_rect);
    }
    // Desenhar soldados na área de embarque (esquerda da tela)
    int soldados_restantes = soldados_total - soldados_resgatados - soldados_embarcados;
    for (int i = 0; i < soldados_restantes && i < 5; i++) {
        int x = 10 + (i * 20);
        int y = SCREEN_HEIGHT - 50;
        SDL_Rect soldado_rect = {x, y, 15, 20};
        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 0, 255); // amarelo
        SDL_RenderFillRect(graphics.renderer, &soldado_rect);
        // Borda do soldado
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // laranja
        SDL_RenderDrawRect(graphics.renderer, &soldado_rect);
    }
    
    // Desenhar soldado(s) embarcados no helicóptero
    if (helicoptero.vivo && soldados_embarcados > 0) {
        int hx = (helicoptero.x * SCREEN_WIDTH) / TELA_LARGURA;
        int hy = (helicoptero.y * SCREEN_HEIGHT) / TELA_ALTURA;
        // Desenhar soldado(s) no helicóptero
        SDL_Rect soldado_rect = {hx + 5, hy - 5, 10, 10};
        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 0, 255); // amarelo
        SDL_RenderFillRect(graphics.renderer, &soldado_rect);
        // Borda do soldado
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // laranja
        SDL_RenderDrawRect(graphics.renderer, &soldado_rect);
    }
    // Desenhar baterias (azul)
    extern Bateria bateria0, bateria1;
    if (bateria0.ativa) {
        int x = (bateria0.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (bateria0.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect bat0_rect = {x, y, 18, 12};
        
        if (bateria0.recarregando) {
            SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // laranja (recarregando)
        } else {
            SDL_SetRenderDrawColor(graphics.renderer, 30, 144, 255, 255); // azul
        }
        SDL_RenderFillRect(graphics.renderer, &bat0_rect);
        // Borda da bateria
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255); // azul escuro
        SDL_RenderDrawRect(graphics.renderer, &bat0_rect);
    }
    if (bateria1.ativa) {
        int x = (bateria1.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (bateria1.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect bat1_rect = {x, y, 18, 12};
        
        if (bateria1.recarregando) {
            SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // laranja (recarregando)
        } else {
            SDL_SetRenderDrawColor(graphics.renderer, 0, 191, 255, 255); // azul claro
        }
        SDL_RenderFillRect(graphics.renderer, &bat1_rect);
        // Borda da bateria
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255); // azul escuro
        SDL_RenderDrawRect(graphics.renderer, &bat1_rect);
    }
    // Desenhar foguetes (vermelho)
    extern Foguete foguetes[10];
    for (int i = 0; i < MAX_FOGUETES; i++) {
        if (foguetes[i].ativo) {
            int x = (foguetes[i].x * SCREEN_WIDTH) / TELA_LARGURA;
            int y = (foguetes[i].y * SCREEN_HEIGHT) / TELA_ALTURA;
            SDL_Rect fog_rect = {x, y, 8, 4};
            SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(graphics.renderer, &fog_rect);
            // Borda do foguete
            SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(graphics.renderer, &fog_rect);
        }
    }
    // UI soldados
    char status[128];
    snprintf(status, sizeof(status), "Embarcados: %d  Resgatados: %d  Total: %d", soldados_embarcados, soldados_resgatados, soldados_total);
    graphics_draw_text_centered(status, 10, NULL, (SDL_Color){255,255,255,255});
    
    // Legenda dos elementos
    graphics_draw_text_centered("Verde=Helicoptero  Amarelo=Soldados  Azul=Baterias  Vermelho=Foguetes  Verde=Plataforma", 30, NULL, (SDL_Color){255,255,255,255});
    // APRESENTAR O FRAME
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_game_over(void) {
    if (!graphics.renderer) return;
    
    // Limpar tela com fundo vermelho
    SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255);
    SDL_RenderClear(graphics.renderer);
    
    // Desenhar título
    graphics_draw_text_centered("GAME OVER", 200, NULL, (SDL_Color){255, 255, 255, 255});
    
    // Desenhar botão de voltar
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 300, 200, 50, "VOLTAR AO MENU", 1);
    
    // APRESENTAR O FRAME
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_victory(void) {
    if (!graphics.renderer) return;
    
    // Limpar tela com fundo dourado
    SDL_SetRenderDrawColor(graphics.renderer, 255, 215, 0, 255);
    SDL_RenderClear(graphics.renderer);
    
    // Desenhar título
    graphics_draw_text_centered("VITORIA!", 200, NULL, (SDL_Color){0, 0, 0, 255});
    
    // Desenhar estatísticas
    char stats[256];
    sprintf(stats, "Soldados resgatados: %d/%d", soldados_resgatados, soldados_total);
    graphics_draw_text_centered(stats, 250, NULL, (SDL_Color){0, 0, 0, 255});
    
    // Desenhar botão de voltar
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 350, 200, 50, "VOLTAR AO MENU", 1);
    
    // APRESENTAR O FRAME
    SDL_RenderPresent(graphics.renderer);
}

int graphics_handle_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                printf("Evento de saída detectado (X da janela)!\n");
                jogo_rodando = 0;
                return 0;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        printf("ESC pressionado - Saindo do jogo\n");
                        jogo_rodando = 0;
                        SDL_Event quit_event;
                        quit_event.type = SDL_QUIT;
                        SDL_PushEvent(&quit_event);
                        return 0;
                        break;
                        
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        printf("ENTER pressionado\n");
                        if (graphics.current_state == MENU_PRINCIPAL) {
                            if (graphics.selected_menu_option == 0) {
                                graphics.current_state = DIFICULDADE;
                            } else if (graphics.selected_menu_option == 1) {
                                graphics.current_state = DIFICULDADE;
                            } else if (graphics.selected_menu_option == 2) {
                                jogo_rodando = 0;
                                SDL_Event quit_event;
                                quit_event.type = SDL_QUIT;
                                SDL_PushEvent(&quit_event);
                                return 0;
                            }
                        } else if (graphics.current_state == DIFICULDADE) {
                            graphics.current_state = JOGO;
                        } else if (graphics.current_state == GAME_OVER || graphics.current_state == VITORIA) {
                            graphics.current_state = MENU_PRINCIPAL;
                        }
                        break;
                        
                    case SDLK_UP:
                        if (graphics.current_state == JOGO) {
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_cima = 1;
                            pthread_mutex_unlock(&helicoptero.mutex);
                        } else if (graphics.current_state == MENU_PRINCIPAL) {
                            if (graphics.selected_menu_option > 0) {
                                graphics.selected_menu_option--;
                            }
                        } else if (graphics.current_state == DIFICULDADE) {
                            if (graphics.selected_difficulty > 1) {
                                graphics.selected_difficulty--;
                            }
                        }
                        break;
                        
                    case SDLK_DOWN:
                        if (graphics.current_state == JOGO) {
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_baixo = 1;
                            pthread_mutex_unlock(&helicoptero.mutex);
                        } else if (graphics.current_state == MENU_PRINCIPAL) {
                            if (graphics.selected_menu_option < 2) {
                                graphics.selected_menu_option++;
                            }
                        } else if (graphics.current_state == DIFICULDADE) {
                            if (graphics.selected_difficulty < 3) {
                                graphics.selected_difficulty++;
                            }
                        }
                        break;
                        
                    case SDLK_LEFT:
                        if (graphics.current_state == JOGO) {
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_esquerda = 1;
                            pthread_mutex_unlock(&helicoptero.mutex);
                        }
                        break;
                        
                    case SDLK_RIGHT:
                        if (graphics.current_state == JOGO) {
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_direita = 1;
                            pthread_mutex_unlock(&helicoptero.mutex);
                        }
                        break;
                        
                    case SDLK_SPACE:
                        if (graphics.current_state == JOGO) {
                            printf("Jogo pausado\n");
                        }
                        break;
                }
                break;
                
            case SDL_KEYUP:
                if (graphics.current_state == JOGO) {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_cima = 0;
                            pthread_mutex_unlock(&helicoptero.mutex);
                            break;
                        case SDLK_DOWN:
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_baixo = 0;
                            pthread_mutex_unlock(&helicoptero.mutex);
                            break;
                        case SDLK_LEFT:
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_esquerda = 0;
                            pthread_mutex_unlock(&helicoptero.mutex);
                            break;
                        case SDLK_RIGHT:
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_direita = 0;
                            pthread_mutex_unlock(&helicoptero.mutex);
                            break;
                    }
                }
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    int mouse_x = event.button.x;
                    int mouse_y = event.button.y;
                    printf("Mouse clicado em: %d, %d\n", mouse_x, mouse_y);
                    
                    if (graphics.current_state == MENU_PRINCIPAL) {
                        if (mouse_x >= SCREEN_WIDTH/2 - 100 && mouse_x <= SCREEN_WIDTH/2 + 100 &&
                            mouse_y >= 200 && mouse_y <= 250) {
                            graphics.current_state = DIFICULDADE;
                        }
                        else if (mouse_x >= SCREEN_WIDTH/2 - 100 && mouse_x <= SCREEN_WIDTH/2 + 100 &&
                                 mouse_y >= 340 && mouse_y <= 390) {
                            jogo_rodando = 0;
                        }
                    } else if (graphics.current_state == DIFICULDADE) {
                        if (mouse_x >= SCREEN_WIDTH/2 - 100 && mouse_x <= SCREEN_WIDTH/2 + 100) {
                            if (mouse_y >= 200 && mouse_y <= 250) {
                                graphics.selected_difficulty = 1;
                                graphics.current_state = JOGO;
                            } else if (mouse_y >= 270 && mouse_y <= 320) {
                                graphics.selected_difficulty = 2;
                                graphics.current_state = JOGO;
                            } else if (mouse_y >= 340 && mouse_y <= 390) {
                                graphics.selected_difficulty = 3;
                                graphics.current_state = JOGO;
                            } else if (mouse_y >= 410 && mouse_y <= 460) {
                                graphics.current_state = MENU_PRINCIPAL;
                            }
                        }
                    }
                }
                break;
        }
    }
    return 1;
}

void graphics_set_state(GameState state) {
    graphics.current_state = state;
}

GameState graphics_get_state(void) {
    return graphics.current_state;
}

int graphics_get_selected_difficulty(void) {
    return graphics.selected_difficulty;
} 