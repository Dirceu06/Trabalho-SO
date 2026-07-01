#ifndef GERENCIADOR_MEMORIA_H
#define GERENCIADOR_MEMORIA_H

#include <vector>
#include <list>
#include <string>

enum class Politica {
    FIFO,
    LRU,
    OTIMO
};

struct EntradaPagina {
    bool  presente;  
    int   frame;      
};

struct Frame {
    bool valido;   
    int  processo_idx;  
    int  pagina;       
    int  tempo_carga;   
    int  ultimo_uso;    
};

struct ResultadoAcesso {
    int  processo_idx;
    int  pagina;
    bool page_fault;     
    int  frame_usado;    
    int  pagina_expulsa;
    int  processo_expulso;
};

class GerenciadorMemoria {
public:

    GerenciadorMemoria(int mem_fisica_mb,
                       int tam_pagina_mb,
                       int num_processos,
                       const std::vector<int>& mem_proc_mb,
                       Politica politica);

    ResultadoAcesso acessar(int processo_idx, int pagina, int tick_atual);

    void set_linha_tempo(int (*lt)[3], int tam);

    int  get_page_faults()  const { return page_faults_total; }
    int  get_num_frames()   const { return static_cast<int>(frames.size()); }
    int  get_tam_pagina()   const { return tam_pagina_mb; }
    Politica get_politica() const { return politica; }
    const std::vector<Frame>& get_frames() const { return frames; }

    int num_paginas(int processo_idx) const;

    std::string dump_frames() const;

private:

    int     tam_pagina_mb;
    Politica politica;

    std::vector<Frame>                    frames;

    std::vector<std::vector<EntradaPagina>> tabela;

    int page_faults_total;

    std::list<int> fila_fifo;

    int (*linha_tempo_otimo)[3];
    int  tam_linha_otimo;

    int  achar_frame_livre() const;
    int  substituir_fifo();
    int  substituir_lru();
    int  substituir_otimo(int tick_atual);

    int  proximo_uso(int processo_idx, int pagina, int a_partir_de) const;
};

#endif // GERENCIADOR_MEMORIA_H