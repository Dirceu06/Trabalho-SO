#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "base.h"

int contar_processos(Processo vetor[]);

float tempo_medio_turnaround(int (*lt)[3], int tam);
float tempo_medio_espera    (int (*lt)[3], int tam);

void imprimir_gantt   (int (*lt)[3], int tam);
void imprimir_metricas(int (*lt)[3], int tam);

int (*rr_linha(int quantum, int *tam_retorno))[3];
int (*sjf_linha(int *tam_retorno))[3];
int (*priori_linha(int *tam_retorno))[3];

#endif