using namespace std;
#include "base.h"
#include "auxiliares.h"
#include <iostream>
#include <cstdlib>

/*
 * Escalonador Round Robin Preemptivo
 * Retorna: array dinâmico int[3] onde cada posição = 1 unidade de tempo
 *   [0] = índice do processo em processos[]  (-1 = CPU ociosa)
 *   [1] = tempo restante DO processo naquela unidade (antes de executar)
 *   [2] = prioridade do processo (mantido por consistência)
 * *tam_retorno = tamanho total da linha do tempo
 *
 * Regra de enfileiramento:
 *   Chegadas no instante t são inseridas ANTES do processo preemptado,
 *   seguindo a convenção mais comum dos livros-texto.
 */
int (*rr_linha(int quantum, int *tam_retorno))[3] {
    int contagem = contar_processos(processos);

    int restante[MAX_PROCESSOS];
    bool concluido[MAX_PROCESSOS];
    for (int i = 0; i < contagem; i++) {
        restante[i] = processos[i].tempo_execucao;
        concluido[i] = false;
    }

    // Fila de prontos: FIFO de índices de processo
    int *fila = NULL;
    int tam_fila = 0;

    // Linha do tempo de retorno
    int (*linha_tempo)[3] = NULL;
    int tam_linha = 0;

    int unit_tempo   = 0;
    int concluidos   = 0;
    int proc_atual   = -1;   // índice do processo na CPU (-1 = ociosa)
    int quantum_usado = 0;   // fatia já consumida pelo proc_atual

    while (concluidos < contagem) {

        // 1. Enfileira processos que chegaram nesta unidade de tempo
        for (int i = 0; i < contagem; i++) {
            if (!concluido[i] && processos[i].tempo_chegada == unit_tempo) {
                tam_fila++;
                fila = (int *)realloc(fila, tam_fila * sizeof(int));
                fila[tam_fila - 1] = i;
            }
        }

        // 2. Preempção por esgotamento do quantum
        //    (processo não terminou, volta para o fim da fila)
        if (proc_atual != -1 && quantum_usado == quantum) {
            tam_fila++;
            fila = (int *)realloc(fila, tam_fila * sizeof(int));
            fila[tam_fila - 1] = proc_atual;
            proc_atual    = -1;
            quantum_usado = 0;
        }

        // 3. Seleciona próximo processo da fila (FIFO)
        if (proc_atual == -1 && tam_fila > 0) {
            proc_atual = fila[0];

            // Remove o primeiro elemento com shift
            for (int i = 0; i < tam_fila - 1; i++)
                fila[i] = fila[i + 1];
            tam_fila--;

            if (tam_fila > 0)
                fila = (int *)realloc(fila, tam_fila * sizeof(int));
            else {
                free(fila);
                fila = NULL;
            }
            quantum_usado = 0;
        }

        // 4. CPU ociosa (nenhum processo pronto)
        if (proc_atual == -1) {
            tam_linha++;
            linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
            linha_tempo[tam_linha - 1][0] = -1;
            linha_tempo[tam_linha - 1][1] = 0;
            linha_tempo[tam_linha - 1][2] = -1;
            unit_tempo++;
            continue;
        }

        // 5. Registra na linha do tempo (tempo restante ANTES de executar)
        tam_linha++;
        linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
        linha_tempo[tam_linha - 1][0] = proc_atual;
        linha_tempo[tam_linha - 1][1] = restante[proc_atual];
        linha_tempo[tam_linha - 1][2] = processos[proc_atual].prioridade;

        // 6. Executa 1 unidade de tempo
        restante[proc_atual]--;
        quantum_usado++;

        // 7. Processo terminou?
        if (restante[proc_atual] == 0) {
            concluido[proc_atual] = true;
            concluidos++;
            proc_atual    = -1;
            quantum_usado = 0;
        }

        unit_tempo++;
    }

    free(fila);
    if (tam_retorno) *tam_retorno = tam_linha;
    return linha_tempo;
}

/*int main() {
    int tam;
    int (*linha)[3] = rr_linha(QUANTUM_PADRAO, &tam);

    printf("=== Round Robin  (quantum = %d) ===\n", QUANTUM_PADRAO);
    imprimir_gantt(linha, tam);
    imprimir_metricas(linha, tam);

    free(linha);
    return 0;
}*/