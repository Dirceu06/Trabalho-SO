// ============================================================
//  teste_memoria.cpp
//  Testa o GerenciadorMemoria de forma isolada (sem Qt, sem A).
//  Compile com:
//    g++ -Wall -Wextra -std=c++17 -g3 teste_memoria.cpp GerenciadorMemoria.cpp -o output/teste_mem
// ============================================================
#include "GerenciadorMemoria.h"
#include <iostream>
#include <cassert>

// Imprime o resultado de um acesso de forma legível
void log_acesso(int tick, const ResultadoAcesso& r) {
    std::cout << "t=" << tick
              << "  P" << r.processo_idx
              << " pag[" << r.pagina << "]"
              << "  frame=" << r.frame_usado;
    if (r.page_fault) {
        std::cout << "  ** PAGE FAULT **";
        if (r.pagina_expulsa != -1)
            std::cout << "  expulsou P" << r.processo_expulso
                      << "/pag[" << r.pagina_expulsa << "]";
    }
    std::cout << "\n";
}

// ============================================================
//  Caso 1 — FIFO com string de referência clássica
//  Frames: 3  |  Referências: 7 0 1 2 0 3 0 4 2 3 0 3 2
//  Page faults esperados (FIFO, 3 frames): 9
// ============================================================
void teste_fifo() {
    std::cout << "====== TESTE FIFO ======\n";

    // 1 processo com 8 "páginas" (simula as referências como páginas)
    std::vector<int> mem = {8 * 64}; // 8 páginas de 64 MB = 512 MB
    GerenciadorMemoria gm(3 * 64, 64, 1, mem, Politica::FIFO); // 3 frames de 64 MB

    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n = sizeof(refs) / sizeof(refs[0]);

    for (int t = 0; t < n; t++) {
        auto r = gm.acessar(0, refs[t], t);
        log_acesso(t, r);
    }

    std::cout << "Page Faults: " << gm.get_page_faults()
              << " (esperado: 10)\n\n";
    assert(gm.get_page_faults() == 10);
}

// ============================================================
//  Caso 2 — LRU com string de referência clássica
//  Frames: 3  |  Referências: 7 0 1 2 0 3 0 4 2 3 0 3 2
//  Page faults esperados (LRU, 3 frames): 8
// ============================================================
void teste_lru() {
    std::cout << "====== TESTE LRU ======\n";

    std::vector<int> mem = {8 * 64};
    GerenciadorMemoria gm(3 * 64, 64, 1, mem, Politica::LRU);

    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n = sizeof(refs) / sizeof(refs[0]);

    for (int t = 0; t < n; t++) {
        auto r = gm.acessar(0, refs[t], t);
        log_acesso(t, r);
    }

    std::cout << "Page Faults: " << gm.get_page_faults()
              << " (esperado: 9)\n\n";
    assert(gm.get_page_faults() == 9);
}

// ============================================================
//  Caso 3 — Ótimo com string de referência clássica
//  Frames: 3  |  Referências: 7 0 1 2 0 3 0 4 2 3 0 3 2
//  Page faults esperados (Ótimo, 3 frames): 6
//
//  Para o Ótimo precisamos da linha do tempo futura.
//  Aqui construímos manualmente um int(*)[3] que representa
//  "todos os ticks são do processo 0, acessando a página refs[t]".
// ============================================================
void teste_otimo() {
    std::cout << "====== TESTE ÓTIMO ======\n";

    int refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n = sizeof(refs) / sizeof(refs[0]);

    // Monta a linha do tempo: [processo_idx, tempo_restante, prioridade]
    // Não usamos tempo_restante nem prioridade no gerenciador de memória.
    int lt[13][3];
    for (int t = 0; t < n; t++) {
        lt[t][0] = 0;   // processo 0
        lt[t][1] = 0;
        lt[t][2] = 0;
    }

    std::vector<int> mem = {8 * 64};
    GerenciadorMemoria gm(3 * 64, 64, 1, mem, Politica::OTIMO);
    gm.set_linha_tempo(lt, n);

    // Para o Ótimo funcionar corretamente com páginas arbitrárias
    // (não cíclicas), precisamos mapear cada tick para a página certa.
    // Como o Ótimo usa "t % npag" internamente, mas aqui as referências
    // não são cíclicas, fazemos o acesso diretamente pela string de referência.
    for (int t = 0; t < n; t++) {
        auto r = gm.acessar(0, refs[t], t);
        log_acesso(t, r);
    }

    std::cout << "Page Faults: " << gm.get_page_faults()
              << " (esperado: 6)\n\n";
    assert(gm.get_page_faults() == 6);
}

// ============================================================
//  Caso 4 — Multi-processo: simula integração real com o A
//  2 processos com memórias diferentes, 4 frames físicos, FIFO
// ============================================================
void teste_multi_processo() {
    std::cout << "====== TESTE MULTI-PROCESSO (FIFO) ======\n";

    // Processo 0: 256 MB → 2 páginas de 128 MB
    // Processo 1: 512 MB → 4 páginas de 128 MB
    std::vector<int> mem = {256, 512};
    GerenciadorMemoria gm(512, 128, 2, mem, Politica::FIFO); // 4 frames de 128 MB

    // Simula uma sequência de acessos que o escalonador faria
    // {processo, pagina, tick}
    int seq[][3] = {
        {0, 0, 0}, {0, 1, 1},
        {1, 0, 2}, {1, 1, 3}, {1, 2, 4}, {1, 3, 5},
        {0, 0, 6}, // HIT esperado
        {1, 0, 7}, // deve causar page fault (todos os frames cheios)
    };

    for (auto& s : seq) {
        auto r = gm.acessar(s[0], s[1], s[2]);
        log_acesso(s[2], r);
    }

    std::cout << gm.dump_frames();
    std::cout << "\n";
}

int main() {
    teste_fifo();
    teste_lru();
    // teste_otimo(); // descomente após confirmar FIFO e LRU
    teste_multi_processo();

    std::cout << "Todos os testes passaram!\n";
    return 0;
}