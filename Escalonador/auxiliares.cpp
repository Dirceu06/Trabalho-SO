using namespace std;
#include <base.h>
#include <exception>
#include <iostream>
#include <base.h>

int contar_processos(Processo vetor[]){
    int contagem = 0;
    for (int i = 0; i < MAX_PROCESSOS; i++) {
        if (vetor[i].tempo_execucao > 0)
            contagem++;
    }
    return contagem;
}