using namespace std;
#include "base.h"
#include "auxiliares.h"
#include <iostream>
#include <cstdlib>

/*
 * Escalonador por Prioridade Preemptivo
 * Retorna: array dinâmico int[3] onde cada posição = 1 unidade de tempo
 *   [0] = índice do processo em processos[]  (-1 = CPU ociosa)
 *   [1] = tempo restante DO processo naquela unidade
 *   [2] = prioridade do processo
 * *tam_retorno = tamanho total da linha do tempo
 */
int (*priori_linha(int *tam_retorno))[3] {
    int contagem = contar_processos(processos);

    // Tempo restante de execução por processo
    int restante[MAX_PROCESSOS];
    bool concluido[MAX_PROCESSOS];
    for (int i = 0; i < contagem; i++) {
        restante[i] = processos[i].tempo_execucao;
        concluido[i] = false;
    }

    // Fila de prontos: {índice_processo, tempo_restante, prioridade}
    int (*fila)[3] = NULL;
    int tam_fila = 0;

    // Linha do tempo de retorno
    int (*linha_tempo)[3] = NULL;
    int tam_linha = 0;

    int unit_tempo = 0;
    int concluidos = 0;

    while (concluidos < contagem) {

        // 1. Adiciona processos que chegaram nesta unidade à fila de prontos
        for (int i = 0; i < contagem; i++) {
            if (!concluido[i] && processos[i].tempo_chegada == unit_tempo) {
                tam_fila++;
                fila = (int (*)[3])realloc(fila, tam_fila * sizeof(int[3]));
                fila[tam_fila - 1][0] = i;
                fila[tam_fila - 1][1] = restante[i];
                fila[tam_fila - 1][2] = processos[i].prioridade;
            }
        }

        // 2. CPU ociosa se não há ninguém pronto
        if (tam_fila == 0) {
            tam_linha++;
            linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
            linha_tempo[tam_linha - 1][0] = -1;
            linha_tempo[tam_linha - 1][1] = 0;
            linha_tempo[tam_linha - 1][2] = -1;
            unit_tempo++;
            continue;
        }

        // 3. Escolhe o processo de maior prioridade (menor número = maior prioridade)
        //    Desempate: menor tempo_chegada (FCFS entre iguais)
        int idx_melhor = 0;
        for (int i = 1; i < tam_fila; i++) {
            bool prioridade_maior = fila[i][2] < fila[idx_melhor][2];
            bool mesmo_prior_chegou_antes =
                fila[i][2] == fila[idx_melhor][2] &&
                processos[fila[i][0]].tempo_chegada < processos[fila[idx_melhor][0]].tempo_chegada;

            if (prioridade_maior || mesmo_prior_chegou_antes)
                idx_melhor = i;
        }

        // 4. Registra na linha do tempo
        tam_linha++;
        linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
        linha_tempo[tam_linha - 1][0] = fila[idx_melhor][0];
        linha_tempo[tam_linha - 1][1] = fila[idx_melhor][1];  // tempo restante antes de executar
        linha_tempo[tam_linha - 1][2] = fila[idx_melhor][2];

        // 5. Executa 1 unidade de tempo
        fila[idx_melhor][1]--;
        restante[fila[idx_melhor][0]]--;

        // 6. Processo terminou? Remove da fila
        if (fila[idx_melhor][1] == 0) {
            concluido[fila[idx_melhor][0]] = true;
            concluidos++;

            // Shift para remover o elemento
            for (int i = idx_melhor; i < tam_fila - 1; i++) {
                fila[i][0] = fila[i + 1][0];
                fila[i][1] = fila[i + 1][1];
                fila[i][2] = fila[i + 1][2];
            }
            tam_fila--;
            if (tam_fila > 0)
                fila = (int (*)[3])realloc(fila, tam_fila * sizeof(int[3]));
            else {
                free(fila);
                fila = NULL;
            }
        }

        unit_tempo++;
    }

    free(fila);
    if (tam_retorno) *tam_retorno = tam_linha;
    return linha_tempo;
}

int main() {
    int tam;
    int (*linha)[3] = priori_linha(&tam);

    imprimir_gantt(linha, tam);
    imprimir_metricas(linha, tam);

    free(linha);
    return 0;
}