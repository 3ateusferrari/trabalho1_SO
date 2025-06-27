# Trabalho 1 - Sistemas Operacionais

## Jogo com pthreads

### Membros do Grupo
- Ricardo Cipriani
- Davi Bonfa
- Mateus Ferrari

### Descrição

Este projeto implementa um jogo simples em C utilizando múltiplas threads (pthreads) para simular a interação entre um helicóptero, baterias de foguetes, obstáculos e sincronização de recursos como ponte e depósito.

#### Threads utilizadas:
- **Thread principal:** Inicializa o jogo, cria as threads e captura a entrada do jogador.
- **Thread do helicóptero:** Atualiza a posição do helicóptero e verifica colisões.
- **Thread das baterias:** Disparam foguetes, recarregam munição e sincronizam travessia da ponte e acesso ao depósito.
- **Thread dos foguetes:** Anima os foguetes e detecta colisão com o helicóptero.
- **Thread do motor do jogo:** Atualiza o estado geral do jogo periodicamente.

#### Sincronização
- Mutexes para ponte, depósito, tela e variáveis de estado.

### Compilação

Entre na pasta `src` e execute:

```
make
```

### Execução

```
./jogo
```

### Limpeza dos arquivos objeto

```
make clean
```

### Observações
- O código está modularizado para facilitar a implementação e manutenção.
- Os detalhes de movimentação, colisão e lógica de jogo devem ser implementados nos arquivos correspondentes.