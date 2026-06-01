# Simulador de Sistemas Operacionais

<h4 align="center">Feito por</h4> 

<div align="center">
    <h3><a href="https://github.com/Dirceu06">Dirceu Jr.</a> ·
    <a href="https://github.com/EduardoHaagP">Eduardo Haag</a> ·
    <a href="https://github.com/lkmotta">Lucas Motta</a></h3>
</div>

<br>

> Projeto acadêmico de Engenharia de Computação — 1º Semestre 2026  
> Simulação integrada de **escalonamento de processos** e **gerência de memória virtual** com interface gráfica em Qt.

---

## Índice

- [Sobre o Projeto](#sobre-o-projeto)
- [Funcionalidades](#funcionalidades)
- [Estrutura do Repositório](#estrutura-do-repositório)
- [Pré-requisitos](#pré-requisitos)
- [Como Compilar e Executar](#como-compilar-e-executar)
- [Formato do Arquivo CSV](#formato-do-arquivo-csv)
- [Algoritmos Implementados](#algoritmos-implementados)
- [Interface Gráfica](#interface-gráfica)
- [Relatório de Saída](#relatório-de-saída)

---

## Sobre o Projeto

Este simulador foi desenvolvido como projeto da disciplina de Sistemas Operacionais, com o objetivo de consolidar os conceitos teóricos de escalonamento de processos e gerência de memória virtual por meio de implementação prática em C++.

A aplicação possui interface gráfica (Qt6), permite carregar processos a partir de um arquivo CSV e exibe um relatório completo ao final da simulação, incluindo diagrama de Gantt e métricas de desempenho.

---

## Funcionalidades

- Carregamento de processos via arquivo **CSV**
- Três algoritmos de **escalonamento preemptivo**
- Três políticas de **substituição de páginas**
- Configuração de memória física e tamanho de página
- **Diagrama de Gantt** visual com marcação de page faults
- **Relatório** com métricas por processo e totais

---

## Estrutura do Repositório

```
projeto-so/
│
├── main.cpp                        # Ponto de entrada da aplicação Qt
├── mainwindow.cpp / .h / .ui       # Tela inicial (seleção do CSV)
├── escalonador.cpp / .h / .ui      # Tela de configuração da simulação
├── relatoriowindow.cpp / .h / .ui  # Tela de relatório e Gantt
├── CMakeLists.txt                  # Build com CMake + Qt6
├── teste.csv                       # Exemplo de arquivo de entrada
│
├── Escalonador/
│   ├── base.h / base.cpp           # Struct Processo e globals
│   ├── auxiliares.h / .cpp         # Métricas e impressão de Gantt
│   ├── rr.cpp                      # Round Robin
│   ├── sjf.cpp                     # SJF Preemptivo (SRTF)
│   └── priori.cpp                  # Prioridade Preemptiva
│
└── Memoria/
    ├── GerenciadorMemoria.h / .cpp  # Gerenciador de memória virtual
    └── teste_memoria.cpp            # Testes isolados do módulo de memória
```

---

## Pré-requisitos

- **C++17** ou superior
- **Qt 6.5+** (módulos `Core` e `Widgets`)
- **CMake 3.19+**
- Compilador compatível: GCC, Clang ou MSVC

### Instalação do Qt (Linux)

```bash
sudo apt install qt6-base-dev cmake build-essential
```

---

## Como Compilar e Executar

### Via CMake (recomendado)

```bash
# Clone o repositório
git clone <url-do-repositorio>
cd projeto-so

# Configure e compile
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Execute
./build/projeto-so
```

### Compilando os módulos do Escalonador isoladamente

```bash
cd Escalonador

g++ priori.cpp auxiliares.cpp -I. -o output/priori.exe
g++ rr.cpp     auxiliares.cpp -I. -o output/rr.exe
g++ sjf.cpp    auxiliares.cpp -I. -o output/sjf.exe
```

### Compilando os testes de memória isoladamente

```bash
cd Memoria

g++ -Wall -Wextra -std=c++17 -g3 \
    teste_memoria.cpp GerenciadorMemoria.cpp \
    -o output/teste_mem

./output/teste_mem
```

---

## Formato do Arquivo CSV

O simulador aceita arquivos `.csv` com **até 10 processos**, sem cabeçalho, no seguinte formato:

```
tempo_chegada, tempo_execucao, prioridade, memoria_mb
```

| Campo            | Descrição                          | Tipo    |
|------------------|------------------------------------|---------|
| `tempo_chegada`  | Instante de chegada do processo    | Inteiro |
| `tempo_execucao` | CPU Burst (deve ser > 0)           | Inteiro |
| `prioridade`     | Prioridade (menor = maior prioridade) | Inteiro |
| `memoria_mb`     | Memória necessária em MB           | Inteiro |

### Exemplo (`teste.csv`)

```csv
0, 2, 3, 256
1, 8, 1, 512
2, 3, 2, 512
3, 5, 3, 256
```

---

## Algoritmos Implementados

### Escalonamento de Processos

| Algoritmo               | Descrição |
|-------------------------|-----------|
| **Round Robin (RR)**    | Preemptivo com quantum configurável via interface. Desempate por ordem de chegada (FCFS). |
| **SJF Preemptivo**      | Shortest Remaining Time First (SRTF). Sempre executa o processo com menor tempo restante. Desempate por chegada. |
| **Prioridade Preemptiva** | Executa o processo com menor número de prioridade. Desempate por tempo de chegada. |

### Substituição de Páginas

| Política  | Descrição |
|-----------|-----------|
| **FIFO**  | Expulsa a página carregada há mais tempo. |
| **LRU**   | Expulsa a página com o acesso mais antigo. |
| **Ótimo** | Expulsa a página que será usada mais tarde no futuro (requer pré-execução para obter a linha do tempo). |

> **Nota sobre o algoritmo Ótimo:** o simulador executa o escalonador duas vezes quando essa política é selecionada: uma passagem prévia para obter a linha do tempo futura e uma segunda passagem com o `GerenciadorMemoria` devidamente configurado.

---

## Interface Gráfica

O simulador possui três telas:

### 1. Tela Inicial
- Botão para seleção do arquivo CSV
- Validação automática do formato do arquivo

### 2. Tela de Configuração
- Seleção do algoritmo de escalonamento (RR, SJF, Prioridade)
- Campo de quantum (visível apenas para Round Robin)
- Configuração de memória física e tamanho de página (em MB)
- Seleção da política de substituição (FIFO, LRU, Ótimo)
- Botão "Gerar Relatório"

### 3. Tela de Relatório
- **Diagrama de Gantt** com cores por processo e marcação visual de page faults (borda vermelha + letra "F")
- **Tabela de métricas** por processo: chegada, tempo de resposta, espera e page faults
- **Resumo global**: Tempo Médio de Resposta, Tempo Médio de Espera e Total de Page Faults
- Botão para gerar nova simulação

---

## Relatório de Saída

Ao final da simulação, o relatório exibe:

- **Linha do Tempo (Gantt):** visualização tick a tick da execução dos processos, com indicação de page faults
- **Tempo de Resposta** por processo: `conclusão - chegada`
- **Tempo de Espera** por processo: `resposta - execução`
- **Tempo Médio de Resposta (TMR)**
- **Tempo Médio de Espera (TME)**
- **Total de Page Faults**

---

