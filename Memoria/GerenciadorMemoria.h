#ifndef GERENCIADOR_MEMORIA_H
#define GERENCIADOR_MEMORIA_H

#include <vector>
#include <list>
#include <string>

// ============================================================
//  Políticas de substituição de páginas disponíveis
// ============================================================
enum class Politica {
    FIFO,
    LRU,
    OTIMO
};

// ============================================================
//  Uma entrada na Tabela de Páginas de UM processo
// ============================================================
struct EntradaPagina {
    bool  presente;   // está em um frame físico agora?
    int   frame;      // qual frame (-1 = não está na RAM)
};

// ============================================================
//  O que está carregado em cada frame físico
// ============================================================
struct Frame {
    bool valido;        // frame está ocupado?
    int  processo_idx;  // a qual processo pertence
    int  pagina;        // qual página do processo
    int  tempo_carga;   // tick em que foi carregado (para FIFO)
    int  ultimo_uso;    // tick do último acesso    (para LRU)
};

// ============================================================
//  Resultado de cada acesso à memória (para log / GUI)
// ============================================================
struct ResultadoAcesso {
    int  processo_idx;
    int  pagina;
    bool page_fault;     // houve falta de página?
    int  frame_usado;    // frame onde a página foi colocada/encontrada
    int  pagina_expulsa; // página expulsa (-1 = nenhuma, havia frame livre)
    int  processo_expulso;
};

// ============================================================
//  Gerenciador de Memória Virtual com Paginação
// ============================================================
class GerenciadorMemoria {
public:
    // mem_fisica_mb   : tamanho total da RAM física (ex: 1024 MB)
    // tam_pagina_mb   : tamanho de cada página/frame  (ex: 256 MB)
    // num_processos   : quantos processos existem no CSV
    // mem_proc_mb     : vetor com a memória necessária de cada processo
    // politica        : FIFO, LRU ou OTIMO
    GerenciadorMemoria(int mem_fisica_mb,
                       int tam_pagina_mb,
                       int num_processos,
                       const std::vector<int>& mem_proc_mb,
                       Politica politica);

    // --------------------------------------------------------
    //  Chamado pelo Escalonador a cada unidade de tempo.
    //  processo_idx : índice do processo que está rodando na CPU
    //  pagina       : número da página que o processo está acessando
    //  tick_atual   : unidade de tempo corrente (necessário p/ LRU/Ótimo)
    //  Retorna o ResultadoAcesso para o Integrante C plotar.
    // --------------------------------------------------------
    ResultadoAcesso acessar(int processo_idx, int pagina, int tick_atual);

    // --------------------------------------------------------
    //  Para o algoritmo Ótimo: deve ser chamado ANTES de iniciar
    //  a simulação, passando a linha do tempo do escalonador.
    //  lt[t][0] = processo_idx no tick t  (-1 = CPU ociosa)
    // --------------------------------------------------------
    void set_linha_tempo(int (*lt)[3], int tam);

    // --------------------------------------------------------
    //  Helpers para o Integrante C exibir na GUI
    // --------------------------------------------------------
    int  get_page_faults()  const { return page_faults_total; }
    int  get_num_frames()   const { return static_cast<int>(frames.size()); }
    const std::vector<Frame>& get_frames() const { return frames; }

    // Número de páginas que um processo ocupa
    int num_paginas(int processo_idx) const;

    // Retorna snapshot textual dos frames (útil para debug no terminal)
    std::string dump_frames() const;

private:
    // -- Configuração --
    int     tam_pagina_mb;
    Politica politica;

    // -- Estado dos frames físicos --
    std::vector<Frame>                    frames;
    // Tabela de páginas: tabela[proc][pag] = EntradaPagina
    std::vector<std::vector<EntradaPagina>> tabela;

    // -- Contadores --
    int page_faults_total;

    // -- Para FIFO: fila com os índices dos frames na ordem de carga --
    std::list<int> fila_fifo;

    // -- Para Ótimo: linha do tempo futura --
    int (*linha_tempo_otimo)[3];
    int  tam_linha_otimo;

    // -- Internos --
    int  achar_frame_livre() const;
    int  substituir_fifo();
    int  substituir_lru();
    int  substituir_otimo(int tick_atual);

    // Próximo uso de um frame no futuro (para Ótimo)
    int  proximo_uso(int processo_idx, int pagina, int a_partir_de) const;
};

#endif // GERENCIADOR_MEMORIA_H