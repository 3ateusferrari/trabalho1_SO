# Jogo do Helicóptero - Versão Gráfica

Este é um jogo de helicóptero desenvolvido em C usando SDL2 para gráficos e pthreads para multithreading.

## Pré-requisitos

### Windows (usando MSYS2/MinGW)

1. Instale o MSYS2: https://www.msys2.org/
2. Abra o terminal MSYS2 e execute:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-SDL2
pacman -S mingw-w64-x86_64-SDL2_ttf
```

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential
sudo apt install libsdl2-dev libsdl2-ttf-dev
```

### macOS

```bash
brew install sdl2 sdl2_ttf
```

## Compilação

1. Navegue até a pasta `src`:
```bash
cd src
```

2. Compile o jogo:
```bash
make
```

3. Execute o jogo:
```bash
./jogo
```

## Controles

- **Setas**: Mover o helicóptero
- **Q** ou **ESC**: Sair do jogo

## Objetivo

O objetivo é resgatar todos os soldados levando-os até a plataforma verde no canto inferior direito da tela, evitando os foguetes vermelhos e os obstáculos cinzas.

## Elementos do Jogo

- **Helicóptero (Ciano)**: O jogador controla este elemento
- **Baterias (Verde)**: Disparam foguetes automaticamente
- **Foguetes (Vermelho)**: Projéteis que destroem o helicóptero
- **Plataforma (Verde escuro)**: Local para desembarcar soldados
- **Obstáculos (Cinza)**: Bloqueiam o movimento

## Dificuldades

1. **Fácil**: 3 foguetes por bateria, recarga em 2 segundos
2. **Médio**: 5 foguetes por bateria, recarga em 1 segundo
3. **Difícil**: 10 foguetes por bateria, recarga em 0.5 segundos

## Estrutura do Projeto

```
src/
├── main.c          # Função principal e thread de renderização
├── graphics.c      # Sistema gráfico SDL2
├── graphics.h      # Cabeçalho do sistema gráfico
├── helicoptero.c   # Lógica do helicóptero
├── helicoptero.h   # Cabeçalho do helicóptero
├── bateria.c       # Lógica das baterias
├── bateria.h       # Cabeçalho das baterias
├── foguete.c       # Lógica dos foguetes
├── foguete.h       # Cabeçalho dos foguetes
├── motor.c         # Motor principal do jogo
├── motor.h         # Cabeçalho do motor
├── utils.c         # Utilitários e estruturas
├── utils.h         # Cabeçalho dos utilitários
└── Makefile        # Script de compilação
```

## Solução de Problemas

### Erro de compilação SDL2
Se você encontrar erros relacionados ao SDL2, certifique-se de que:
1. O SDL2 está instalado corretamente
2. As variáveis de ambiente estão configuradas
3. O `sdl2-config` está no PATH

### Erro de fonte
Se o jogo não conseguir carregar a fonte, ele tentará usar fontes padrão do sistema. Em alguns sistemas, pode ser necessário instalar fontes adicionais.

## Limpeza

Para limpar os arquivos compilados:
```bash
make clean
```