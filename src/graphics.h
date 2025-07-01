#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include "utils.h"

// Constantes da tela gráfica
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define CELL_SIZE 16
#define GRID_WIDTH (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)

// Estados do jogo
typedef enum {
    MENU_PRINCIPAL,
    DIFICULDADE,
    JOGO,
    GAME_OVER,
    VITORIA
} GameState;

// Estrutura para o sistema gráfico
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int initialized;
    GameState current_state;
    int selected_difficulty;
    int selected_menu_option; // 0=JOGAR, 1=DIFICULDADE, 2=SAIR
} GraphicsSystem;

// Variável global do sistema gráfico
extern GraphicsSystem graphics;

// Funções de inicialização e limpeza
int graphics_init(void);
void graphics_cleanup(void);

// Funções de renderização
void graphics_draw_ui(int soldados_embarcados, int soldados_resgatados, int soldados_total, int bateria_atual);
void graphics_draw_game_over(void);
void graphics_draw_victory(void);

// Funções de interface
void graphics_draw_main_menu(void);
void graphics_draw_difficulty_menu(void);
void graphics_draw_game_screen(void);
void graphics_draw_button(int x, int y, int width, int height, const char* text, int selected);
void graphics_draw_text_centered(const char* text, int y, void* font, SDL_Color color);

// Funções de entrada
int graphics_handle_events(void);

// Funções de estado
void graphics_set_state(GameState state);
GameState graphics_get_state(void);
int graphics_get_selected_difficulty(void);

#endif // GRAPHICS_H 