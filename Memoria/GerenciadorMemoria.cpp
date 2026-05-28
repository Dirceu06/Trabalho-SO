#include "GerenciadorMemoria.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <climits>

// ============================================================
//  Construtor
// ============================================================
GerenciadorMemoria::GerenciadorMemoria(int mem_fisica_mb,
                                       int tam_pagina_mb,
                                       int num_processos,
                                       const std::vector<int>& mem_proc_mb,
                                       Politica politica)
    : tam_pagina_mb(tam_pagina_mb),
      politica(politica),
      page_faults_total(0),
      linha_tempo_otimo(nullptr),
      tam_linha_otimo(0)
{
    if (tam_pagina_mb <= 0)
        throw std::invalid_argument("Tamanho de pagina deve ser > 0");

    // Cria os frames físicos (todos livres no início)
    int num_frames = mem_fisica_mb / tam_pagina_mb;
    frames.resize(num_frames, {false, -1, -1, 0, 0});

    // Cria a tabela de páginas para cada processo
    tabela.resize(num_processos);
    for (int p = 0; p < num_processos; p++) {
        int npag = static_cast<int>(std::ceil(
            static_cast<double>(mem_proc_mb[p]) / tam_pagina_mb));
        tabela[p].resize(npag, {false, -1});
    }
}

// ============================================================
//  Registra a linha do tempo (necessária para o algoritmo Ótimo)
// ============================================================
void GerenciadorMemoria::set_linha_tempo(int (*lt)[3], int tam) {
    linha_tempo_otimo = lt;
    tam_linha_otimo   = tam;
}

// ============================================================
//  num_paginas — quantas páginas um processo ocupa
// ============================================================
int GerenciadorMemoria::num_paginas(int processo_idx) const {
    return static_cast<int>(tabela[processo_idx].size());
}

// ============================================================
//  acessar — ponto de entrada chamado pelo Escalonador
// ============================================================
ResultadoAcesso GerenciadorMemoria::acessar(int processo_idx, int pagina, int tick_atual) {
    ResultadoAcesso resultado;
    resultado.processo_idx    = processo_idx;
    resultado.pagina          = pagina;
    resultado.page_fault      = false;
    resultado.frame_usado     = -1;
    resultado.pagina_expulsa  = -1;
    resultado.processo_expulso = -1;

    // Valida índices
    if (processo_idx < 0 || processo_idx >= static_cast<int>(tabela.size()))
        throw std::out_of_range("processo_idx invalido");
    if (pagina < 0 || pagina >= static_cast<int>(tabela[processo_idx].size()))
        throw std::out_of_range("pagina invalida para este processo");

    EntradaPagina& entrada = tabela[processo_idx][pagina];

    // ---- HIT: página já está em um frame físico ----
    if (entrada.presente) {
        int f = entrada.frame;
        frames[f].ultimo_uso = tick_atual;   // atualiza LRU
        resultado.frame_usado = f;
        return resultado;
    }

    // ---- PAGE FAULT ----
    page_faults_total++;
    resultado.page_fault = true;

    // 1. Tenta achar um frame livre
    int frame_escolhido = achar_frame_livre();

    // 2. Se não há frame livre, aplica a política de substituição
    if (frame_escolhido == -1) {
        switch (politica) {
            case Politica::FIFO:  frame_escolhido = substituir_fifo();              break;
            case Politica::LRU:   frame_escolhido = substituir_lru();               break;
            case Politica::OTIMO: frame_escolhido = substituir_otimo(tick_atual);   break;
        }

        // Registra o que foi expulso (para o log)
        resultado.pagina_expulsa   = frames[frame_escolhido].pagina;
        resultado.processo_expulso = frames[frame_escolhido].processo_idx;

        // Invalida a entrada antiga na tabela de páginas
        tabela[resultado.processo_expulso][resultado.pagina_expulsa].presente = false;
        tabela[resultado.processo_expulso][resultado.pagina_expulsa].frame    = -1;
    }

    // 3. Carrega a nova página no frame escolhido
    frames[frame_escolhido] = {true, processo_idx, pagina, tick_atual, tick_atual};
    entrada.presente = true;
    entrada.frame    = frame_escolhido;
    resultado.frame_usado = frame_escolhido;

    // 4. Atualiza a fila FIFO (adiciona o frame ao final)
    if (politica == Politica::FIFO) {
        fila_fifo.push_back(frame_escolhido);
    }

    return resultado;
}

// ============================================================
//  achar_frame_livre — retorna índice do primeiro frame vago
//  (-1 se todos estão ocupados)
// ============================================================
int GerenciadorMemoria::achar_frame_livre() const {
    for (int i = 0; i < static_cast<int>(frames.size()); i++) {
        if (!frames[i].valido)
            return i;
    }
    return -1;
}

// ============================================================
//  FIFO — expulsa o frame que foi carregado há mais tempo
// ============================================================
int GerenciadorMemoria::substituir_fifo() {
    // A fila_fifo mantém a ordem de inserção dos frames
    int frame_vitima = fila_fifo.front();
    fila_fifo.pop_front();
    return frame_vitima;
}

// ============================================================
//  LRU — expulsa o frame com o menor tick de último acesso
// ============================================================
int GerenciadorMemoria::substituir_lru() {
    int vitima    = -1;
    int menor_uso = INT_MAX;

    for (int i = 0; i < static_cast<int>(frames.size()); i++) {
        if (frames[i].valido && frames[i].ultimo_uso < menor_uso) {
            menor_uso = frames[i].ultimo_uso;
            vitima    = i;
        }
    }
    return vitima;
}

// ============================================================
//  Ótimo — expulsa o frame cuja página será usada
//           mais tarde no futuro (ou nunca mais usada)
// ============================================================
int GerenciadorMemoria::substituir_otimo(int tick_atual) {
    int vitima         = -1;
    int maior_distancia = -1;

    for (int f = 0; f < static_cast<int>(frames.size()); f++) {
        if (!frames[f].valido) continue;

        int dist = proximo_uso(frames[f].processo_idx, frames[f].pagina, tick_atual + 1);

        if (dist > maior_distancia) {
            maior_distancia = dist;
            vitima          = f;
        }
    }
    return vitima;
}

// ============================================================
//  proximo_uso — quantos ticks até essa página ser acessada
//               novamente (INT_MAX = não será mais acessada)
// ============================================================
int GerenciadorMemoria::proximo_uso(int processo_idx, int pagina, int a_partir_de) const {
    if (!linha_tempo_otimo)
        return INT_MAX; // linha do tempo não fornecida → nunca usa

    for (int t = a_partir_de; t < tam_linha_otimo; t++) {
        if (linha_tempo_otimo[t][0] != processo_idx)
            continue;

        // Calcula qual página o processo acessa naquele tick
        // (mesma fórmula usada em acessar() — ciclo pelas páginas)
        int npag = static_cast<int>(tabela[processo_idx].size());
        int pag_futura = t % npag;   // política simples de acesso sequencial cíclico

        if (pag_futura == pagina)
            return t - a_partir_de;
    }

    return INT_MAX; // não será mais acessada → candidata ideal à expulsão
}

// ============================================================
//  dump_frames — snapshot textual para debug
// ============================================================
std::string GerenciadorMemoria::dump_frames() const {
    std::ostringstream oss;
    oss << "Estado dos Frames:\n";
    for (int i = 0; i < static_cast<int>(frames.size()); i++) {
        oss << "  Frame[" << i << "]: ";
        if (!frames[i].valido) {
            oss << "[LIVRE]\n";
        } else {
            oss << "Proc=" << frames[i].processo_idx
                << " Pag="  << frames[i].pagina
                << " carga=" << frames[i].tempo_carga
                << " uso="   << frames[i].ultimo_uso << "\n";
        }
    }
    oss << "  Page Faults acumulados: " << page_faults_total << "\n";
    return oss.str();
}