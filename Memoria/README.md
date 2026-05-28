# Módulo B — GerenciadorMemoria

## Arquivos
```
Memoria/
├── GerenciadorMemoria.h     ← interface pública (Integrante C vai incluir isso)
├── GerenciadorMemoria.cpp   ← implementação dos 3 algoritmos
└── teste_memoria.cpp        ← testes isolados (sem Qt, sem Escalonador)
```

## Compilar e testar de forma isolada

```bash
g++ -Wall -Wextra -std=c++17 -g3 \
    teste_memoria.cpp GerenciadorMemoria.cpp \
    -o output/teste_mem

./output/teste_mem
```

## Como o Integrante A (Escalonador) chama seu código

No loop principal do escalonador, **a cada unidade de tempo**, após
decidir qual processo roda, A chama:

```cpp
#include "../Memoria/GerenciadorMemoria.h"

// Instancia uma vez antes do loop (feito pelo Integrante C na MainWindow):
// GerenciadorMemoria gm(mem_fisica, tam_pagina, num_procs, mem_procs, politica);

// Dentro do loop de simulação:
int tick = unit_tempo;           // variável já existente em priori.cpp
int proc = fila[idx_melhor][0]; // índice do processo rodando

// Calcula qual página o processo acessa neste tick
// (cíclico entre as páginas do processo — modelo simples)
int npag  = gm.num_paginas(proc);
int pagina = tick % npag;

ResultadoAcesso r = gm.acessar(proc, pagina, tick);

// r.page_fault        → true se houve falta de página
// r.frame_usado       → frame onde a página está
// r.pagina_expulsa    → qual página foi expulsa (-1 se havia frame livre)
// gm.get_page_faults()→ contador total de page faults
```

## Como o Integrante C (GUI) usa os resultados

```cpp
// Depois que a simulação terminar:
int total_pf = gm.get_page_faults();
// Plotar no relatório

// Para visualização em tempo real (QTimer tick a tick):
// guardar o vetor de ResultadoAcesso e iterar sobre ele
```

## Algoritmos implementados

| Política | Estratégia |
|---|---|
| `Politica::FIFO` | Expulsa o frame carregado há mais tempo |
| `Politica::LRU`  | Expulsa o frame com o acesso mais antigo |
| `Politica::OTIMO`| Expulsa o frame que será usado mais tarde (requer `set_linha_tempo`) |

## Para o algoritmo Ótimo

O algoritmo Ótimo precisa "ver o futuro". Por isso, antes de iniciar
a simulação o Integrante C deve chamar:

```cpp
// lt = linha do tempo gerada pelo escalonador (int(*)[3])
gm.set_linha_tempo(lt, tam_lt);
```