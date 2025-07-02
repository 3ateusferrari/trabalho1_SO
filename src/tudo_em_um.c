#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// --- Definitions from utils.h ---
#define TELA_LARGURA 80
#define TELA_ALTURA 24
#define MAX_OBSTACULOS 10
#define MAX_FOGUETES 10
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define CELL_SIZE 16
#define GRID_WIDTH (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)

typedef enum {
    FACIL = 1,
    MEDIO = 2,
    DIFICIL = 3
} Dificuldade;

typedef enum {
    MENU_PRINCIPAL,
    DIFICULDADE,
    JOGO,
    GAME_OVER,
    VITORIA
} GameState;

typedef struct {
    int x, y;
    int ativo;
} Obstaculo;

typedef struct {
    int x, y;
    int largura, altura;
} Plataforma;

typedef struct {
    int x, y;
    int vivo;
    int movendo_cima, movendo_baixo, movendo_esquerda, movendo_direita;
    pthread_mutex_t mutex;
} Helicoptero;

typedef struct {
    int x, y;
    int ativo;
    pthread_mutex_t mutex;
} Foguete;

typedef struct {
    int id;
    int municao;
    int recarregando;
    int ativa;
    int x, y;
    int nivel;
    int movimento_x, movimento_y; // Direção do movimento
    int timer_movimento; // Timer para mudança de direção
    pthread_mutex_t mutex;
} Bateria;

typedef struct {
    int bateria_atual;
    pthread_mutex_t mutex;
} Motor;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    int initialized;
    GameState current_state;
    int selected_difficulty;
    int selected_menu_option;
} GraphicsSystem;

int capacidade_bateria = 5;
useconds_t tempo_recarga_bateria = 1000000;
useconds_t tempo_disparo_min = 300000;
useconds_t tempo_disparo_max = 500000;
Dificuldade nivel_dificuldade = MEDIO;
int soldados_embarcados = 1;
int soldados_resgatados = 0;
int soldados_total = 10;
int jogo_rodando = 1;

pthread_mutex_t mutex_ponte = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deposito = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_recarga = PTHREAD_MUTEX_INITIALIZER;

char tela[TELA_ALTURA][TELA_LARGURA+1];
Obstaculo obstaculos[MAX_OBSTACULOS];
Plataforma plataforma;
Helicoptero helicoptero;
Foguete foguetes[10];
Bateria bateria0, bateria1;
Motor motor;
GraphicsSystem graphics = {0};

// --- Funções utilitárias ---
void configurar_dificuldade(Dificuldade dificuldade) {
    nivel_dificuldade = dificuldade;
    switch (dificuldade) {
        case FACIL:
            capacidade_bateria = 2;
            tempo_recarga_bateria = 2000000;
            tempo_disparo_min = 800000;
            tempo_disparo_max = 1500000;
            break;
        case MEDIO:
            capacidade_bateria = 5;
            tempo_recarga_bateria = 800000;
            tempo_disparo_min = 400000;
            tempo_disparo_max = 800000;
            break;
        case DIFICIL:
            capacidade_bateria = 10;
            tempo_recarga_bateria = 300000;
            tempo_disparo_min = 200000;
            tempo_disparo_max = 400000;
            break;
    }
    printf("Dificuldade configurada: %d (capacidade: %d, recarga: %d ms, disparo: %d-%d ms)\n", 
           dificuldade, capacidade_bateria, (int)(tempo_recarga_bateria/1000), 
           (int)(tempo_disparo_min/1000), (int)(tempo_disparo_max/1000));
}

// --- Funções de lógica e threads ---
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

int verificar_colisao_foguetes() {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&foguetes[i].mutex);
        if (foguetes[i].ativo && 
            foguetes[i].x == helicoptero.x && 
            foguetes[i].y == helicoptero.y) {
            pthread_mutex_unlock(&foguetes[i].mutex);
            return 1;
        }
        pthread_mutex_unlock(&foguetes[i].mutex);
    }
    return 0;
}

void* thread_func_helicoptero(void* arg) {
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
        if (helicoptero.movendo_cima && helicoptero.y > 0) helicoptero.y--;
        if (helicoptero.movendo_baixo && helicoptero.y < TELA_ALTURA - 1) helicoptero.y++;
        if (helicoptero.movendo_esquerda && helicoptero.x > 0) helicoptero.x--;
        if (helicoptero.movendo_direita && helicoptero.x < TELA_LARGURA - 1) helicoptero.x++; // Fixed typo
        if (verificar_colisao_foguetes()) { // Fixed function name
            helicoptero.vivo = 0;
            printf("Helicóptero atingido por foguete!\n");
        }
        if (helicoptero.y <= 0) {
            helicoptero.vivo = 0;
            printf("Helicóptero explodiu ao atingir o topo!\n");
        }
        pthread_mutex_unlock(&helicoptero.mutex);
        usleep(50000);
    }
    return NULL;
}

void* thread_func_foguetes(void* arg) {
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
                foguetes[i].x--;
                if (foguetes[i].x < 0) foguetes[i].ativo = 0;
            }
            pthread_mutex_unlock(&foguetes[i].mutex);
        }
        usleep(100000);
    }
    return NULL;
}

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
    return -1;
}

void mover_bateria_ate(Bateria* bat, int destino_x, int destino_y) {
    while (bat->x != destino_x || bat->y != destino_y) {
        pthread_mutex_lock(&bat->mutex);
        if (bat->x < destino_x) bat->x++;
        else if (bat->x > destino_x) bat->x--;
        if (bat->y < destino_y) bat->y++;
        else if (bat->y > destino_y) bat->y--;
        pthread_mutex_unlock(&bat->mutex);
        usleep(50000);
    }
}

void recarregar_bateria(Bateria* bat) {
    pthread_mutex_lock(&mutex_recarga);
    int pos_x = bat->x;
    int pos_y = bat->y;
    int ponte_x = 5;
    int ponte_y = bat->y;
    int deposito_x = 2;
    int deposito_y = TELA_ALTURA - 2;
    mover_bateria_ate(bat, ponte_x, ponte_y);
    pthread_mutex_lock(&mutex_ponte);
    usleep(200000);
    pthread_mutex_unlock(&mutex_ponte);
    mover_bateria_ate(bat, deposito_x, deposito_y);
    pthread_mutex_lock(&mutex_deposito);
    usleep(tempo_recarga_bateria);
    bat->municao = capacidade_bateria;
    bat->nivel = capacidade_bateria;
    bat->recarregando = 0;
    pthread_mutex_unlock(&mutex_deposito);
    mover_bateria_ate(bat, ponte_x, ponte_y);
    pthread_mutex_lock(&mutex_ponte);
    usleep(200000);
    pthread_mutex_unlock(&mutex_ponte);
    mover_bateria_ate(bat, pos_x, pos_y);
    pthread_mutex_unlock(&mutex_recarga);
}

void* thread_func_bateria0(void* arg) {
    bateria0.id = 0;
    bateria0.municao = capacidade_bateria;
    bateria0.recarregando = 0;
    bateria0.ativa = 1;
    bateria0.x = TELA_LARGURA - 5;
    bateria0.y = 5;
    bateria0.nivel = capacidade_bateria;
    pthread_mutex_init(&bateria0.mutex, NULL);
    usleep(100000);
    while (jogo_rodando) {
        pthread_mutex_lock(&bateria0.mutex);
        if (bateria0.municao > 0 && !bateria0.recarregando) {
            int foguete_id = encontrar_foguete_livre();
            if (foguete_id != -1) {
                foguetes[foguete_id].x = bateria0.x;
                foguetes[foguete_id].y = bateria0.y;
                bateria0.municao--;
                bateria0.nivel = bateria0.municao;
            }
        } else if (bateria0.municao == 0 && !bateria0.recarregando) {
            bateria0.recarregando = 1;
            pthread_mutex_unlock(&bateria0.mutex);
            recarregar_bateria(&bateria0);
            continue;
        }
        pthread_mutex_unlock(&bateria0.mutex);
        usleep(tempo_disparo_min + (rand() % (tempo_disparo_max - tempo_disparo_min)));
    }
    return NULL;
}

void* thread_func_bateria1(void* arg) {
    bateria1.id = 1;
    bateria1.municao = capacidade_bateria;
    bateria1.recarregando = 0;
    bateria1.ativa = 1;
    bateria1.x = TELA_LARGURA - 10;
    bateria1.y = 10;
    bateria1.nivel = capacidade_bateria;
    pthread_mutex_init(&bateria1.mutex, NULL);
    usleep(150000);
    while (jogo_rodando) {
        pthread_mutex_lock(&bateria1.mutex);
        if (bateria1.municao > 0 && !bateria1.recarregando) {
            int foguete_id = encontrar_foguete_livre();
            if (foguete_id != -1) {
                foguetes[foguete_id].x = bateria1.x;
                foguetes[foguete_id].y = bateria1.y;
                bateria1.municao--;
                bateria1.nivel = bateria1.municao;
            }
        } else if (bateria1.municao == 0 && !bateria1.recarregando) {
            bateria1.recarregando = 1;
            pthread_mutex_unlock(&bateria1.mutex);
            recarregar_bateria(&bateria1);
            continue;
        }
        pthread_mutex_unlock(&bateria1.mutex);
        usleep(tempo_disparo_min + (rand() % (tempo_disparo_max - tempo_disparo_min)) + 100000);
    }
    return NULL;
}

void reinicializar_baterias(void) {
    pthread_mutex_lock(&bateria0.mutex);
    bateria0.municao = capacidade_bateria;
    bateria0.nivel = capacidade_bateria;
    bateria0.recarregando = 0;
    pthread_mutex_unlock(&bateria0.mutex);
    pthread_mutex_lock(&bateria1.mutex);
    bateria1.municao = capacidade_bateria;
    bateria1.nivel = capacidade_bateria;
    bateria1.recarregando = 0;
    pthread_mutex_unlock(&bateria1.mutex);
    printf("Baterias reinicializadas com %d foguetes cada\n", capacidade_bateria);
}

void inicializar_obstaculos_e_plataforma() {
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        obstaculos[i].x = 20 + i * 5;
        obstaculos[i].y = 5 + (i % 10);
        obstaculos[i].ativo = 1;
    }
    plataforma.x = TELA_LARGURA - 8;
    plataforma.y = TELA_ALTURA - 3;
    plataforma.largura = 6;
    plataforma.altura = 2;
}

int colisao_com_obstaculo(int x, int y) {
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo && obstaculos[i].x == x && obstaculos[i].y == y)
            return 1;
    }
    return 0;
}

int helicoptero_na_plataforma() {
    int hx, hy;
    pthread_mutex_lock(&helicoptero.mutex);
    hx = helicoptero.x;
    hy = helicoptero.y;
    pthread_mutex_unlock(&helicoptero.mutex);
    if (hx >= plataforma.x && hx < plataforma.x + plataforma.largura &&
        hy >= plataforma.y && hy < plataforma.y + plataforma.altura)
        return 1;
    return 0;
}

int colisao_com_estruturas(int x, int y) {
    if ((x == TELA_LARGURA - 5 && y == 5) ||
        (x == TELA_LARGURA - 10 && y == 10))
        return 1;
    if (x <= 5 && y >= TELA_ALTURA - 3)
        return 1;
    if (x >= plataforma.x && x < plataforma.x + plataforma.largura &&
        y >= plataforma.y && y < plataforma.y + plataforma.altura)
        return 2;
    return 0;
}

void* thread_func_motor(void* arg) {
    motor.bateria_atual = 100;
    pthread_mutex_init(&motor.mutex, NULL);
    inicializar_obstaculos_e_plataforma();
    resetar_helicoptero();
    while (jogo_rodando) {
        pthread_mutex_lock(&helicoptero.mutex);
        if (!helicoptero.vivo) {
            pthread_mutex_unlock(&helicoptero.mutex);
            jogo_rodando = 0;
            printf("Game Over! Helicóptero destruído!\n");
            break;
        }
        if (helicoptero.y >= TELA_ALTURA - 1) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com o solo!\n");
        }
        if (colisao_com_obstaculo(helicoptero.x, helicoptero.y)) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com obstáculo!\n");
        }
        int col = colisao_com_estruturas(helicoptero.x, helicoptero.y);
        if (col == 1) {
            helicoptero.vivo = 0;
            printf("Helicóptero colidiu com estrutura!\n");
        }
        if (col == 2 && helicoptero.vivo && soldados_embarcados > 0) {
            soldados_resgatados += soldados_embarcados;
            printf("Soldados resgatados! Total: %d/%d\n", soldados_resgatados, soldados_total);
            if (soldados_resgatados >= soldados_total) {
                jogo_rodando = 0;
                printf("VITÓRIA! Todos os soldados foram resgatados!\n");
                pthread_mutex_unlock(&helicoptero.mutex);
                break;
            } else {
                resetar_helicoptero();
            }
        }
        pthread_mutex_unlock(&helicoptero.mutex);
        usleep(150000);
    }
    return NULL;
}

// Função para desenhar helicóptero mais bonito
void graphics_draw_helicopter(int x, int y) {
    if (!graphics.renderer) return;
    
    // Corpo principal (azul claro)
    SDL_Rect corpo = {x, y + 5, 25, 8};
    SDL_SetRenderDrawColor(graphics.renderer, 100, 149, 237, 255);
    SDL_RenderFillRect(graphics.renderer, &corpo);
    
    // Cockpit (azul escuro)
    SDL_Rect cockpit = {x + 18, y + 3, 7, 6};
    SDL_SetRenderDrawColor(graphics.renderer, 25, 25, 112, 255);
    SDL_RenderFillRect(graphics.renderer, &cockpit);
    
    // Rotor principal (circular)
    SDL_SetRenderDrawColor(graphics.renderer, 64, 64, 64, 255);
    for(int i = 0; i < 360; i += 30) {
        int rx = x + 12 + (int)(15 * cos(i * M_PI / 180));
        int ry = y + 2 + (int)(2 * sin(i * M_PI / 180));
        SDL_RenderDrawLine(graphics.renderer, x + 12, y + 2, rx, ry);
    }
    
    // Cauda
    SDL_Rect cauda = {x - 8, y + 7, 12, 3};
    SDL_SetRenderDrawColor(graphics.renderer, 100, 149, 237, 255);
    SDL_RenderFillRect(graphics.renderer, &cauda);
    
    // Rotor de cauda
    SDL_SetRenderDrawColor(graphics.renderer, 64, 64, 64, 255);
    SDL_RenderDrawLine(graphics.renderer, x - 10, y + 4, x - 10, y + 12);
    SDL_RenderDrawLine(graphics.renderer, x - 13, y + 8, x - 7, y + 8);
}

// Função para desenhar bateria/canhão mais bonito
void graphics_draw_battery(int x, int y, int recarregando) {
    if (!graphics.renderer) return;
    
    // Base da bateria (verde militar)
    SDL_Rect base = {x, y + 8, 20, 10};
    if (recarregando) {
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255); // Laranja quando recarregando
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 85, 107, 47, 255); // Verde militar
    }
    SDL_RenderFillRect(graphics.renderer, &base);
    
    // Torre da bateria
    SDL_Rect torre = {x + 6, y + 4, 8, 8};
    SDL_SetRenderDrawColor(graphics.renderer, 105, 105, 105, 255); // Cinza
    SDL_RenderFillRect(graphics.renderer, &torre);
    
    // Canhão
    SDL_Rect canhao = {x + 14, y + 6, 12, 4};
    SDL_SetRenderDrawColor(graphics.renderer, 64, 64, 64, 255); // Cinza escuro
    SDL_RenderFillRect(graphics.renderer, &canhao);
    
    // Detalhes da bateria
    SDL_SetRenderDrawColor(graphics.renderer, 169, 169, 169, 255);
    SDL_RenderDrawRect(graphics.renderer, &base);
    SDL_RenderDrawRect(graphics.renderer, &torre);
}

// Função para desenhar foguete mais bonito
void graphics_draw_rocket(int x, int y) {
    if (!graphics.renderer) return;
    
    // Corpo do foguete (vermelho)
    SDL_Rect corpo = {x, y, 12, 4};
    SDL_SetRenderDrawColor(graphics.renderer, 220, 20, 60, 255);
    SDL_RenderFillRect(graphics.renderer, &corpo);
    
    // Ponta do foguete (amarelo)
    SDL_Rect ponta = {x + 12, y + 1, 4, 2};
    SDL_SetRenderDrawColor(graphics.renderer, 255, 215, 0, 255);
    SDL_RenderFillRect(graphics.renderer, &ponta);
    
    // Rastro do foguete (laranja)
    SDL_SetRenderDrawColor(graphics.renderer, 255, 140, 0, 255);
    SDL_RenderDrawLine(graphics.renderer, x - 2, y + 1, x, y + 1);
    SDL_RenderDrawLine(graphics.renderer, x - 2, y + 2, x, y + 2);
    SDL_RenderDrawLine(graphics.renderer, x - 4, y + 2, x - 2, y + 2);
}

// Função para desenhar soldado mais bonito
void graphics_draw_soldier(int x, int y, int on_helicopter) {
    if (!graphics.renderer) return;
    
    if (on_helicopter) {
        // Soldado menor quando no helicóptero
        SDL_Rect capacete = {x, y, 6, 4};
        SDL_SetRenderDrawColor(graphics.renderer, 34, 139, 34, 255); // Verde
        SDL_RenderFillRect(graphics.renderer, &capacete);
        
        SDL_Rect corpo = {x + 1, y + 4, 4, 6};
        SDL_SetRenderDrawColor(graphics.renderer, 107, 142, 35, 255); // Verde oliva
        SDL_RenderFillRect(graphics.renderer, &corpo);
    } else {
        // Soldado normal no chão
        SDL_Rect capacete = {x + 2, y, 8, 6};
        SDL_SetRenderDrawColor(graphics.renderer, 34, 139, 34, 255); // Verde
        SDL_RenderFillRect(graphics.renderer, &capacete);
        
        SDL_Rect corpo = {x, y + 6, 12, 10};
        SDL_SetRenderDrawColor(graphics.renderer, 107, 142, 35, 255); // Verde oliva
        SDL_RenderFillRect(graphics.renderer, &corpo);
        
        // Pernas
        SDL_Rect perna1 = {x + 2, y + 16, 3, 6};
        SDL_Rect perna2 = {x + 7, y + 16, 3, 6};
        SDL_SetRenderDrawColor(graphics.renderer, 85, 107, 47, 255);
        SDL_RenderFillRect(graphics.renderer, &perna1);
        SDL_RenderFillRect(graphics.renderer, &perna2);
    }
}

// Função para desenhar plataforma mais bonita
void graphics_draw_platform(int x, int y, int width, int height) {
    if (!graphics.renderer) return;
    
    // Base da plataforma (verde escuro)
    SDL_Rect base = {x, y, width, height};
    SDL_SetRenderDrawColor(graphics.renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(graphics.renderer, &base);
    
    // Bordas da plataforma
    SDL_SetRenderDrawColor(graphics.renderer, 0, 100, 0, 255);
    SDL_RenderDrawRect(graphics.renderer, &base);
    
    // Detalhes da plataforma (listras)
    SDL_SetRenderDrawColor(graphics.renderer, 124, 252, 0, 255);
    for(int i = 0; i < width; i += 8) {
        SDL_RenderDrawLine(graphics.renderer, x + i, y, x + i, y + height);
    }
}

// Função para desenhar obstáculos mais bonitos
void graphics_draw_obstacle(int x, int y) {
    if (!graphics.renderer) return;
    
    int cell_w = SCREEN_WIDTH / TELA_LARGURA;
    int cell_h = SCREEN_HEIGHT / TELA_ALTURA;
    
    // Rocha/obstáculo (cinza com textura)
    SDL_Rect obstaculo = {x, y, cell_w, cell_h};
    SDL_SetRenderDrawColor(graphics.renderer, 105, 105, 105, 255);
    SDL_RenderFillRect(graphics.renderer, &obstaculo);
    
    // Textura do obstáculo
    SDL_SetRenderDrawColor(graphics.renderer, 169, 169, 169, 255);
    SDL_RenderDrawRect(graphics.renderer, &obstaculo);
    
    // Detalhes da rocha
    SDL_SetRenderDrawColor(graphics.renderer, 128, 128, 128, 255);
    SDL_RenderDrawLine(graphics.renderer, x + 2, y + 2, x + cell_w - 2, y + cell_h - 2);
    SDL_RenderDrawLine(graphics.renderer, x + cell_w - 2, y + 2, x + 2, y + cell_h - 2);
}
int graphics_init(void) {
    printf("Iniciando SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    printf("SDL inicializado com sucesso!\n");

    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return 0;
    }
    printf("SDL_ttf inicializado com sucesso!\n");

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

    graphics.font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!graphics.font) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return 0;
    }
    printf("Fonte carregada com sucesso!\n");

    SDL_ShowWindow(graphics.window);
    SDL_RaiseWindow(graphics.window);
    SDL_SetWindowFullscreen(graphics.window, 0);
    SDL_RestoreWindow(graphics.window);
    graphics.current_state = MENU_PRINCIPAL;
    graphics.selected_difficulty = 2;
    graphics.selected_menu_option = 0;
    graphics.initialized = 1;
    printf("Sistema gráfico inicializado com sucesso!\n");
    return 1;
}

void graphics_cleanup(void) {
    printf("Limpando sistema gráfico...\n");
    if (graphics.font) {
        TTF_CloseFont(graphics.font);
        graphics.font = NULL;
    }
    if (graphics.renderer) {
        SDL_DestroyRenderer(graphics.renderer);
        graphics.renderer = NULL;
    }
    if (graphics.window) {
        SDL_DestroyWindow(graphics.window);
        graphics.window = NULL;
    }
    TTF_Quit();
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
    if (!graphics.renderer || !graphics.font) return;

    SDL_Surface* surface = TTF_RenderText_Solid(graphics.font, text, color);
    if (!surface) {
        printf("Failed to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(graphics.renderer, surface);
    if (!texture) {
        printf("Failed to create texture from surface! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    int text_width, text_height;
    SDL_QueryTexture(texture, NULL, NULL, &text_width, &text_height);

    int x = (SCREEN_WIDTH - text_width) / 2;
    SDL_Rect render_rect = {x, y, text_width, text_height};

    SDL_RenderCopy(graphics.renderer, texture, NULL, &render_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void graphics_draw_button(int x, int y, int width, int height, const char* text, int selected) {
    if (!graphics.renderer) return;
    SDL_Rect button_rect = {x, y, width, height};
    if (selected) {
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 100, 100, 100, 255);
    }
    SDL_RenderFillRect(graphics.renderer, &button_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &button_rect);

    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(graphics.font, text, text_color);
    if (!surface) {
        printf("Failed to render button text! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(graphics.renderer, surface);
    if (!texture) {
        printf("Failed to create texture for button text! SDL_Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    int text_width, text_height;
    SDL_QueryTexture(texture, NULL, NULL, &text_width, &text_height);
    SDL_Rect text_rect = {x + (width - text_width) / 2, y + (height - text_height) / 2, text_width, text_height};
    SDL_RenderCopy(graphics.renderer, texture, NULL, &text_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void graphics_draw_main_menu(void) {
    if (!graphics.renderer) return;
    SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255);
    SDL_RenderClear(graphics.renderer);
    graphics_draw_text_centered("HELICOPTERO GAME", 100, NULL, (SDL_Color){255, 255, 255, 255});
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 200, 200, 50, "JOGAR", graphics.selected_menu_option == 0);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 270, 200, 50, "DIFICULDADE", graphics.selected_menu_option == 1);
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 340, 200, 50, "SAIR", graphics.selected_menu_option == 2);
    graphics_draw_text_centered("Use SETAS para navegar, ENTER para selecionar", 450, NULL, (SDL_Color){255, 255, 255, 255});
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_difficulty_menu(void) {
    if (!graphics.renderer) return;
    SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 100, 255);
    SDL_RenderClear(graphics.renderer);
    graphics_draw_text_centered("ESCOLHA A DIFICULDADE", 60, NULL, (SDL_Color){255, 255, 255, 255});
    SDL_Rect easy_rect = {SCREEN_WIDTH/2 - 150, 150, 300, 60};
    if (graphics.selected_difficulty == 1) {
        SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 100, 100, 100, 255);
    }
    SDL_RenderFillRect(graphics.renderer, &easy_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &easy_rect);
    graphics_draw_text_centered("FACIL - 2 municoes, recarga lenta", 170, NULL, (SDL_Color){255, 255, 255, 255});
    SDL_Rect medium_rect = {SCREEN_WIDTH/2 - 150, 230, 300, 60};
    if (graphics.selected_difficulty == 2) {
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 100, 100, 100, 255);
    }
    SDL_RenderFillRect(graphics.renderer, &medium_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &medium_rect);
    graphics_draw_text_centered("MEDIO - 5 municoes, recarga media", 250, NULL, (SDL_Color){255, 255, 255, 255});
    SDL_Rect hard_rect = {SCREEN_WIDTH/2 - 150, 310, 300, 60};
    if (graphics.selected_difficulty == 3) {
        SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(graphics.renderer, 100, 100, 100, 255);
    }
    SDL_RenderFillRect(graphics.renderer, &hard_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &hard_rect);
    graphics_draw_text_centered("DIFICIL - 10 municoes, recarga rapida", 330, NULL, (SDL_Color){255, 255, 255, 255});
    SDL_Rect back_rect = {SCREEN_WIDTH/2 - 150, 390, 300, 50};
    SDL_SetRenderDrawColor(graphics.renderer, 80, 80, 80, 255);
    SDL_RenderFillRect(graphics.renderer, &back_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(graphics.renderer, &back_rect);
    graphics_draw_text_centered("VOLTAR", 405, NULL, (SDL_Color){255, 255, 255, 255});
    graphics_draw_text_centered("Use SETAS para navegar, ENTER para selecionar", 480, NULL, (SDL_Color){200, 200, 255, 255});
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_game_screen(void) {
    if (!graphics.renderer) return;
    SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 0, 255);
    SDL_RenderClear(graphics.renderer);
    SDL_Rect solo_rect = {0, SCREEN_HEIGHT - (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH, SCREEN_HEIGHT / TELA_ALTURA};
    SDL_SetRenderDrawColor(graphics.renderer, 139, 69, 19, 255);
    SDL_RenderFillRect(graphics.renderer, &solo_rect);
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            int x = (obstaculos[i].x * SCREEN_WIDTH) / TELA_LARGURA;
            int y = (obstaculos[i].y * SCREEN_HEIGHT) / TELA_ALTURA;
            SDL_Rect obs_rect = {x, y, SCREEN_WIDTH / TELA_LARGURA, SCREEN_HEIGHT / TELA_ALTURA};
            SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255);
            SDL_RenderDrawRect(graphics.renderer, &obs_rect);
        }
    }
    SDL_Rect deposito_rect = {5, SCREEN_HEIGHT - 3 * (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH / 16, 2 * (SCREEN_HEIGHT / TELA_ALTURA)};
    SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255);
    SDL_RenderDrawRect(graphics.renderer, &deposito_rect);
    SDL_Rect ponte_rect = {SCREEN_WIDTH / 16, SCREEN_HEIGHT - (SCREEN_HEIGHT / TELA_ALTURA), SCREEN_WIDTH / 3, SCREEN_HEIGHT / (2 * TELA_ALTURA)};
    SDL_SetRenderDrawColor(graphics.renderer, 205, 133, 63, 255);
    SDL_RenderFillRect(graphics.renderer, &ponte_rect);
    int plat_x = (plataforma.x * SCREEN_WIDTH) / TELA_LARGURA;
    int plat_y = (plataforma.y * SCREEN_HEIGHT) / TELA_ALTURA;
    SDL_Rect plat_rect = {plat_x, plat_y, (plataforma.largura * SCREEN_WIDTH) / TELA_LARGURA, (plataforma.altura * SCREEN_HEIGHT) / TELA_ALTURA};
    SDL_SetRenderDrawColor(graphics.renderer, 34, 139, 34, 255);
    SDL_RenderFillRect(graphics.renderer, &plat_rect);
    SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(graphics.renderer, &plat_rect);
    pthread_mutex_lock(&helicoptero.mutex);
    if (helicoptero.vivo) {
        int x = (helicoptero.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (helicoptero.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect heli_rect = {x, y, 20, 15};
        SDL_SetRenderDrawColor(graphics.renderer, 173, 255, 47, 255);
        SDL_RenderFillRect(graphics.renderer, &heli_rect);
        SDL_SetRenderDrawColor(graphics.renderer, 0, 255, 0, 255);
        SDL_RenderDrawRect(graphics.renderer, &heli_rect);
    }
    pthread_mutex_unlock(&helicoptero.mutex);
    int soldados_restantes = soldados_total - soldados_resgatados - soldados_embarcados;
    for (int i = 0; i < soldados_restantes && i < 5; i++) {
        int x = 10 + (i * 20);
        int y = SCREEN_HEIGHT - 50;
        SDL_Rect soldado_rect = {x, y, 15, 20};
        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(graphics.renderer, &soldado_rect);
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
        SDL_RenderDrawRect(graphics.renderer, &soldado_rect);
    }
    pthread_mutex_lock(&helicoptero.mutex);
    if (helicoptero.vivo && soldados_embarcados > 0) {
        int hx = (helicoptero.x * SCREEN_WIDTH) / TELA_LARGURA;
        int hy = (helicoptero.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect soldado_rect = {hx + 5, hy - 5, 10, 10};
        SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(graphics.renderer, &soldado_rect);
        SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
        SDL_RenderDrawRect(graphics.renderer, &soldado_rect);
    }
    pthread_mutex_unlock(&helicoptero.mutex);
    if (bateria0.ativa) {
        int x = (bateria0.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (bateria0.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect bat0_rect = {x, y, 18, 12};
        if (bateria0.recarregando) {
            SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
        } else {
            SDL_SetRenderDrawColor(graphics.renderer, 30, 144, 255, 255);
        }
        SDL_RenderFillRect(graphics.renderer, &bat0_rect);
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255);
        SDL_RenderDrawRect(graphics.renderer, &bat0_rect);
    }
    if (bateria1.ativa) {
        int x = (bateria1.x * SCREEN_WIDTH) / TELA_LARGURA;
        int y = (bateria1.y * SCREEN_HEIGHT) / TELA_ALTURA;
        SDL_Rect bat1_rect = {x, y, 18, 12};
        if (bateria1.recarregando) {
            SDL_SetRenderDrawColor(graphics.renderer, 255, 165, 0, 255);
        } else {
            SDL_SetRenderDrawColor(graphics.renderer, 0, 191, 255, 255);
        }
        SDL_RenderFillRect(graphics.renderer, &bat1_rect);
        SDL_SetRenderDrawColor(graphics.renderer, 0, 0, 255, 255);
        SDL_RenderDrawRect(graphics.renderer, &bat1_rect);
    }
    for (int i = 0; i < MAX_FOGUETES; i++) {
        if (foguetes[i].ativo) {
            int x = (foguetes[i].x * SCREEN_WIDTH) / TELA_LARGURA;
            int y = (foguetes[i].y * SCREEN_HEIGHT) / TELA_ALTURA;
            SDL_Rect fog_rect = {x, y, 8, 4};
            SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(graphics.renderer, &fog_rect);
            SDL_SetRenderDrawColor(graphics.renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(graphics.renderer, &fog_rect);
        }
    }
    char status[128];
    snprintf(status, sizeof(status), "Embarcados: %d  Resgatados: %d  Total: %d", soldados_embarcados, soldados_resgatados, soldados_total);
    graphics_draw_text_centered(status, 10, NULL, (SDL_Color){255,255,255,255});
    graphics_draw_text_centered("Verde=Helicoptero  Amarelo=Soldados  Azul=Baterias  Vermelho=Foguetes  Verde=Plataforma", 30, NULL, (SDL_Color){255,255,255,255});
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_game_over(void) {
    if (!graphics.renderer) return;
    SDL_SetRenderDrawColor(graphics.renderer, 255, 0, 0, 255);
    SDL_RenderClear(graphics.renderer);
    graphics_draw_text_centered("GAME OVER", 200, NULL, (SDL_Color){255, 255, 255, 255});
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 300, 200, 50, "VOLTAR AO MENU", 1);
    SDL_RenderPresent(graphics.renderer);
}

void graphics_draw_victory(void) {
    if (!graphics.renderer) return;
    SDL_SetRenderDrawColor(graphics.renderer, 255, 215, 0, 255);
    SDL_RenderClear(graphics.renderer);
    graphics_draw_text_centered("VITORIA!", 200, NULL, (SDL_Color){0, 0, 0, 255});
    char stats[256];
    sprintf(stats, "Soldados resgatados: %d/%d", soldados_resgatados, soldados_total);
    graphics_draw_text_centered(stats, 250, NULL, (SDL_Color){0, 0, 0, 255});
    graphics_draw_button(SCREEN_WIDTH/2 - 100, 350, 200, 50, "VOLTAR AO MENU", 1);
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
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        printf("ESC pressionado - Saindo do jogo\n");
                        jogo_rodando = 0;
                        SDL_Event quit_event;
                        quit_event.type = SDL_QUIT;
                        SDL_PushEvent(&quit_event);
                        return 0;
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
                            if (graphics.selected_menu_option > 0) graphics.selected_menu_option--;
                        } else if (graphics.current_state == DIFICULDADE) {
                            if (graphics.selected_difficulty > 1) graphics.selected_difficulty--;
                        }
                        break;
                    case SDLK_DOWN:
                        if (graphics.current_state == JOGO) {
                            pthread_mutex_lock(&helicoptero.mutex);
                            helicoptero.movendo_baixo = 1;
                            pthread_mutex_unlock(&helicoptero.mutex);
                        } else if (graphics.current_state == MENU_PRINCIPAL) {
                            if (graphics.selected_menu_option < 2) graphics.selected_menu_option++;
                        } else if (graphics.current_state == DIFICULDADE) {
                            if (graphics.selected_difficulty < 3) graphics.selected_difficulty++;
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

// --- Função principal ---
int main() {
    srand(time(NULL));
    soldados_embarcados = 1;
    soldados_resgatados = 0;
    soldados_total = 10;
    jogo_rodando = 1;
    if (!graphics_init()) {
        printf("Erro ao inicializar sistema gráfico!\n");
        return 1;
    }
    printf("Jogo iniciado! Use a interface gráfica para jogar.\n");
    printf("Pressione ESC para sair\n");
    pthread_t thread_helicoptero = 0, thread_bateria = 0, thread_foguete = 0, thread_motor = 0, thread_bateria1 = 0;
    int threads_criadas = 0;
    while (jogo_rodando) {
        graphics_handle_events();
        GameState current_state = graphics_get_state();
        switch (current_state) {
            case MENU_PRINCIPAL:
                graphics_draw_main_menu();
                break;
            case DIFICULDADE:
                graphics_draw_difficulty_menu();
                break;
            case JOGO:
                if (!threads_criadas) {
                    configurar_dificuldade(graphics_get_selected_difficulty());
                    printf("Dificuldade configurada: %d\n", nivel_dificuldade);
                    soldados_embarcados = 1;
                    soldados_resgatados = 0;
                    jogo_rodando = 1;
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
        if (!helicoptero.vivo && current_state == JOGO) {
            graphics_set_state(GAME_OVER);
        } else if (soldados_resgatados >= soldados_total && current_state == JOGO) {
            graphics_set_state(VITORIA);
        }
        if (current_state == MENU_PRINCIPAL && threads_criadas) {
            threads_criadas = 0;
            printf("Jogo resetado - threads serão recriadas na próxima partida\n");
        }
        SDL_Delay(50);
    }
    if (threads_criadas) {
        pthread_join(thread_helicoptero, NULL);
        pthread_join(thread_bateria, NULL);
        pthread_join(thread_bateria1, NULL);
        pthread_join(thread_foguete, NULL);
        pthread_join(thread_motor, NULL);
        printf("Todas as threads finalizadas!\n");
    }
    graphics_cleanup();
    printf("Jogo finalizado!\n");
    return 0;
}