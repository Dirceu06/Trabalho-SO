using namespace std;
#include "base.h"
#include "auxiliares.h"
#include <iostream>
#include <cstdlib>
#include "../Memoria/GerenciadorMemoria.h"

/*
 * Escalonador SJF Preemptivo
 * Retorna: array dinâmico int[3] onde cada posição = 1 unidade de tempo
 *   [0] = índice do processo em processos[]  (-1 = CPU ociosa)
 *   [1] = tempo restante DO processo naquela unidade (antes de executar)
 *   [2] = memoria do processo
 * *tam_retorno = tamanho total da linha do tempo
 */
int (*sjf_linha(int *tam_retorno, GerenciadorMemoria *gm))[3] {
    int contagem = contar_processos(processos);

    // Tempo restante de execução por processo
    int restante[MAX_PROCESSOS];
    bool concluido[MAX_PROCESSOS];
    for (int i = 0; i < contagem; i++) {
        restante[i] = processos[i].tempo_execucao;
        concluido[i] = false;
    }

    // Fila de prontos: {índice_processo, tempo_restante, memoria}
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
                fila[tam_fila - 1][2] = processos[i].memoria;
            }
        }

        // 2. CPU ociosa
        if (tam_fila == 0) {
            tam_linha++;
            linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
            linha_tempo[tam_linha - 1][0] = -1;
            linha_tempo[tam_linha - 1][1] = 0;
            linha_tempo[tam_linha - 1][2] = -1;
            unit_tempo++;
            continue;
        }

        // 3. Escolhe o processo com menor tempo RESTANTE (SRTF)
        //    Desempate: menor tempo_chegada (FCFS entre iguais)
        int idx_melhor = 0;
        for (int i = 1; i < tam_fila; i++) {
            bool menor_restante = fila[i][1] < fila[idx_melhor][1];
            bool mesmo_restante_chegou_antes =
                fila[i][1] == fila[idx_melhor][1] &&
                processos[fila[i][0]].tempo_chegada <
                processos[fila[idx_melhor][0]].tempo_chegada;

            if (menor_restante || mesmo_restante_chegou_antes)
                idx_melhor = i;
        }

        // 4. Registra na linha do tempo
        tam_linha++;
        linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
        linha_tempo[tam_linha - 1][0] = fila[idx_melhor][0];
        linha_tempo[tam_linha - 1][1] = fila[idx_melhor][1]; // tempo restante antes de executar
        linha_tempo[tam_linha - 1][2] = fila[idx_melhor][2];

        // acesso à memória
        if (gm != nullptr) {
            int proc   = fila[idx_melhor][0];
            int pagina = unit_tempo % gm->num_paginas(proc);
            gm->acessar(proc, pagina, unit_tempo);
        }

        // 5. Executa 1 unidade de tempo
        fila[idx_melhor][1]--;
        restante[fila[idx_melhor][0]]--;

        // 6. Processo terminou? Remove da fila
        if (fila[idx_melhor][1] == 0) {
            concluido[fila[idx_melhor][0]] = true;
            concluidos++;

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

/*int main() {
    int contagem = contar_processos(processos);

    // Memória necessária por processo (vem do campo memoria em base.h)
    int mem_procs[MAX_PROCESSOS];
    for (int i = 0; i < contagem; i++)
        mem_procs[i] = processos[i].memoria;

    int mem_fisica  = 1024;   // MB de RAM física
    int tam_pagina  = 256;    // MB por página

    // -------------------------------------------------------
    // FIFO ou LRU: direto
    // -------------------------------------------------------
    GerenciadorMemoria gm(mem_fisica, tam_pagina, contagem, mem_procs, Politica::FIFO);

    int tam;
    int (*linha)[3] = priori_linha(&tam, &gm);   // ou rr_linha / sjf_linha

    imprimir_gantt(linha, tam);
    imprimir_metricas(linha, tam);
    printf("Page faults: %d\n", gm.get_page_faults());

    free(linha);

    // -------------------------------------------------------
    // ÓTIMO: roda o escalonador sem memória primeiro pra ter
    // a linha do tempo, depois refaz com gm
    // -------------------------------------------------------

    int tam_prev;
    int (*lt_prev)[3] = priori_linha(&tam_prev, nullptr);  // 1ª passagem sem memória

    GerenciadorMemoria gm_ot(mem_fisica, tam_pagina, contagem, mem_procs, Politica::OTIMO);
    gm_ot.set_linha_tempo(lt_prev, tam_prev);
    free(lt_prev);

    int (*linha_ot)[3] = priori_linha(&tam_prev, &gm_ot);  // 2ª passagem com memória

    imprimir_gantt(linha_ot, tam_prev);
    imprimir_metricas(linha_ot, tam_prev);
    printf("Page faults (Ótimo): %d\n", gm_ot.get_page_faults());

    free(linha_ot);

    return 0;
}*/