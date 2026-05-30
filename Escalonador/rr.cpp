using namespace std;
#include "base.h"
#include "auxiliares.h"
#include <iostream>
#include <cstdlib>
#include "../Memoria/GerenciadorMemoria.h"

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
int (*rr_linha(int quantum, int *tam_retorno, GerenciadorMemoria *gm))[3] {
    int contagem = contar_processos(processos);

    int restante[MAX_PROCESSOS];
    bool concluido[MAX_PROCESSOS];
    for (int i = 0; i < contagem; i++) {
        restante[i] = processos[i].tempo_execucao;
        concluido[i] = false;
    }

    //fila de prontos: FIFO de índices de processo
    int *fila = NULL;
    int tam_fila = 0;

    int (*linha_tempo)[3] = NULL;
    int tam_linha = 0;

    int unit_tempo   = 0;
    int concluidos   = 0;
    int proc_atual   = -1; 
    int quantum_usado = 0;   // fatia já consumida pelo proc_atual

    while (concluidos < contagem) {

        //Enfileira processos que chegaram nesta unidade de tempo
        for (int i = 0; i < contagem; i++) {
            if (!concluido[i] && processos[i].tempo_chegada == unit_tempo) {
                tam_fila++;
                fila = (int *)realloc(fila, tam_fila * sizeof(int));
                fila[tam_fila - 1] = i;
            }
        }

        //Preempção por esgotamento do quantum
        //    (processo não terminou, volta para o fim da fila)
        if (proc_atual != -1 && quantum_usado == quantum) {
            tam_fila++;
            fila = (int *)realloc(fila, tam_fila * sizeof(int));
            fila[tam_fila - 1] = proc_atual;
            proc_atual    = -1;
            quantum_usado = 0;
        }

        //Seleciona próximo processo da fila (FIFO)
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

        //CPU ociosa
        if (proc_atual == -1) {
            tam_linha++;
            linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
            linha_tempo[tam_linha - 1][0] = -1;
            linha_tempo[tam_linha - 1][1] = 0;
            linha_tempo[tam_linha - 1][2] = -1;
            unit_tempo++;
            continue;
        }

        //Registra na linha do tempo (tempo restante ANTES de executar)
        tam_linha++;
        linha_tempo = (int (*)[3])realloc(linha_tempo, tam_linha * sizeof(int[3]));
        linha_tempo[tam_linha - 1][0] = proc_atual;
        linha_tempo[tam_linha - 1][1] = restante[proc_atual];
        linha_tempo[tam_linha - 1][2] = processos[proc_atual].prioridade;

        //acesso à memória
        if (gm != nullptr) {
            int pagina = unit_tempo % gm->num_paginas(proc_atual);
            gm->acessar(proc_atual, pagina, unit_tempo);
        }

        //Executa 1 unidade de tempo
        restante[proc_atual]--;
        quantum_usado++;

        //Processo terminou?
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
