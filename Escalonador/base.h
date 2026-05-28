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

// Vetor de processos utilizado pelos escalonadores
extern Processo processos[MAX_PROCESSOS];

// Quantum global disponível para Round Robin
extern int QUANTUM_GLOBAL;

#endif // BASE_H
