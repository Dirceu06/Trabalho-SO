using namespace std;
#include "base.h"
#include "auxiliares.h"
#include <cstdio>
#include <cstdlib>

int contar_processos(Processo vetor[]) {
    int contagem = 0;
    for (int i = 0; i < MAX_PROCESSOS; i++)
        if (vetor[i].tempo_execucao > 0)
            contagem++;
    return contagem;
}

float tempo_medio_turnaround(int (*lt)[3], int tam) {
    int contagem = contar_processos(processos);
    float soma = 0;
    for (int p = 0; p < contagem; p++) {
        int conclusao = 0;
        for (int t = tam - 1; t >= 0; t--)
            if (lt[t][0] == p) { conclusao = t + 1; break; }
        soma += conclusao - processos[p].tempo_chegada;
    }
    return soma / contagem;
}

float tempo_medio_espera(int (*lt)[3], int tam) {
    int contagem = contar_processos(processos);
    float soma = 0;
    for (int p = 0; p < contagem; p++) {
        int conclusao = 0;
        for (int t = tam - 1; t >= 0; t--)
            if (lt[t][0] == p) { conclusao = t + 1; break; }
        int turnaround = conclusao - processos[p].tempo_chegada;
        soma += turnaround - processos[p].tempo_execucao;
    }
    return soma / contagem;
}

void imprimir_gantt(int (*lt)[3], int tam) {
    char nomes[] = "ABCDEFGHIJ";
    int contagem = contar_processos(processos);

    printf("     ");
    for (int t = 0; t <= tam; t++) printf("%-3d", t);
    printf("\n");

    for (int p = 0; p < contagem; p++) {
        printf("  %c  ", nomes[p]);
        for (int t = 0; t < tam; t++)
            printf(lt[t][0] == p ? "-- " : "   ");
        printf("\n");
    }

    printf("     ");
    for (int t = 0; t <= tam; t++) printf("%-3d", t);
    printf("\n");
    fflush(stdin);
}

void imprimir_metricas(int (*lt)[3], int tam) {
    char nomes[] = "ABCDEFGHIJ";
    int contagem = contar_processos(processos);

    printf("\n%-10s %-10s %-12s %-10s\n", "Processo", "Chegada", "Resposta", "Espera");
    printf("%-10s %-10s %-12s %-10s\n",   "--------", "-------", "--------", "------");

    for (int p = 0; p < contagem; p++) {
        int conclusao = 0;
        for (int t = tam - 1; t >= 0; t--)
            if (lt[t][0] == p) { conclusao = t + 1; break; }
        int resposta = conclusao - processos[p].tempo_chegada;
        int espera   = resposta  - processos[p].tempo_execucao;
        printf("%-10c %-10d %-12d %-10d\n", nomes[p], processos[p].tempo_chegada, resposta, espera);
    }

    printf("\nTempo médio de resposta : %.2f\n", tempo_medio_turnaround(lt, tam));
    printf("Tempo médio de espera   : %.2f\n",   tempo_medio_espera(lt, tam));
    fflush(stdin);
}