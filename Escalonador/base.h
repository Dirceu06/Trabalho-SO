#ifndef BASE_H
#define BASE_H

#include <stdlib.h>

typedef struct {
    int tempo_chegada;      // Tempo de Chegada
    int tempo_execucao;     // Tempo de Execução (CPU Burst)
    int prioridade;         // Prioridade
    int memoria;            // Memória Necessária (em MB)
} Processo;

// Número máximo de processos
#define MAX_PROCESSOS 10

// Vetor de processos com dados padrão
static Processo processos[MAX_PROCESSOS] = {
    {0, 2, 3, 256}, // A
    {1, 8, 1, 512}, // B
    {2, 3, 2, 512}, // C
    {3, 5, 3, 256} // D
};

#endif // BASE_H
