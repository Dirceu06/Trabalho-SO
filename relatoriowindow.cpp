#include "relatoriowindow.h"
#include "ui_relatoriowindow.h"
#include "Escalonador/auxiliares.h"

#include <QScrollArea>
#include <QHeaderView>
#include <QPainter>
#include <QFont>
#include <QTimer>
#include <cstdlib>

//  paleta de cores pros processos (A–J)
const QColor GanttWidget::CORES_PROC[10] = {
    QColor(100, 180, 255),  // A azul claro
    QColor(120, 210, 120),  // B verde
    QColor(255, 210,  80),  // C amarelo
    QColor(220, 130, 220),  // D roxo
    QColor(255, 150,  80),  // E laranja
    QColor( 80, 210, 210),  // F ciano
    QColor(255, 120, 120),  // G vermelho claro
    QColor(160, 200,  80),  // H verde claro
    QColor(200, 160, 255),  // I roxo claro
    QColor(255, 200, 160),  // J pessego
};

GanttWidget::GanttWidget(QWidget *parent):
    QWidget(parent)
{}

void GanttWidget::setDados(int (*lt)[3], int t, int nProcs,
                           const std::vector<bool> &faults)
{
    linhaTempo = lt;
    tam = t;
    numProcs = nProcs;
    faultPorTick = faults;
    int largura = MARGIN_LEFT + tam * CELL_W + 40;
    int altura = MARGIN_TOP  + numProcs * CELL_H + 44;
    setMinimumSize(largura, altura);
    update();
}

void GanttWidget::paintEvent(QPaintEvent *)
{
    if (!linhaTempo || tam == 0) return;
 
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
 
    QFont fontNormal;
    fontNormal.setPointSize(8);
    p.setFont(fontNormal);
 
    const char nomes[] = "ABCDEFGHIJ";
 
    // eixo superior (numeros de tempo)
    p.setPen(Qt::black);
    for (int t = 0; t <= tam; t++)
        p.drawText(MARGIN_LEFT + t * CELL_W, MARGIN_TOP - 6, QString::number(t));
 
    // labels dos processos na esquerda
    for (int proc = 0; proc < numProcs; proc++)
        p.drawText(6, MARGIN_TOP + proc * CELL_H + CELL_H - 8, QString(nomes[proc]));
 
    // células
    for (int t = 0; t < tam; t++) {
        int  proc = linhaTempo[t][0];
        int  x = MARGIN_LEFT + t * CELL_W;
        bool fault = (t < static_cast<int>(faultPorTick.size())) && faultPorTick[t];
 
        for (int row = 0; row < numProcs; row++) {
            int y = MARGIN_TOP + row * CELL_H;
 
            if (proc == row) {
                // célula ativa: cor do processo
                p.fillRect(x, y, CELL_W - 1, CELL_H - 1, CORES_PROC[row % 10]);
 
                if (fault) {
                    // borda vermelha + letra F = page fault
                    p.setPen(QPen(Qt::red, 2));
                    p.drawRect(x, y, CELL_W - 1, CELL_H - 1);
                    QFont fF = fontNormal;
                    fF.setBold(true);
                    fF.setPointSize(7);
                    p.setFont(fF);
                    p.setPen(Qt::red);
                    p.drawText(x + CELL_W - 10, y + 10, "F");
                    p.setFont(fontNormal);
                } else {
                    p.setPen(QPen(Qt::darkGray, 1));
                    p.drawRect(x, y, CELL_W - 1, CELL_H - 1);
                }
            } else {
                // célula ociosa
                p.fillRect(x, y, CELL_W - 1, CELL_H - 1, QColor(240, 240, 240));
                p.setPen(QPen(Qt::lightGray, 1));
                p.drawRect(x, y, CELL_W - 1, CELL_H - 1);
            }
        }
    }
 
    // eixo inferior
    p.setPen(Qt::black);
    p.setFont(fontNormal);
    int yBottom = MARGIN_TOP + numProcs * CELL_H + 6;
    for (int t = 0; t <= tam; t++){
        p.drawText(MARGIN_LEFT + t * CELL_W, yBottom + 10, QString::number(t));
    }

    // legenda
    int yLeg = yBottom + 22;
    p.fillRect(MARGIN_LEFT, yLeg, 12, 12, QColor(100, 180, 255));
    p.setPen(QPen(Qt::red, 2));
    p.drawRect(MARGIN_LEFT, yLeg, 12, 12);
    p.setPen(Qt::black);
    p.drawText(MARGIN_LEFT + 16, yLeg + 11, "= Page Fault");
}


// construtor da tela
relatorioWindow::relatorioWindow(int (*linha)[3], int tam,
                                 GerenciadorMemoria *gm,
                                 const std::vector<int> &memProcs,
                                 QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::relatorioWindow),
    linhaTempo(linha),
    tamLinha(tam),
    gerMem(gm),
    ganttWidget(nullptr)
{
    ui->setupUi(this);
 
    int numProcs = contar_processos(processos);
    faultsPorProc.assign(numProcs, 0);
 
    // coleta page faults por tick e por processo no clone do GerenciadorMemoria
    coletarFaults(memProcs);
 
    ganttWidget = new GanttWidget();
    ganttWidget->setDados(linhaTempo, tamLinha, numProcs, faultPorTick);
 
    QScrollArea *scroll = new QScrollArea(ui->centralwidget);
    scroll->setWidget(ganttWidget);
    scroll->setWidgetResizable(false);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setGeometry(50, 45, 700, 195);
 
    // tabela de métricas
    ui->tabelaMetricas->setGeometry(50, 245, 700, 195);
 
    // preenche
    preencherTabela();
    preencherLabels();
}

relatorioWindow::~relatorioWindow()
{
    delete ui;
    if (linhaTempo) free(linhaTempo);
    delete gerMem;
}

//  coleta Faults
//  cria um GerenciadorMemoria com as mesmas configurações do original,
//  percorre a linha do tempo e coleta page_fault por tick e por processo.
void relatorioWindow::coletarFaults(const std::vector<int> &memProcs)
{
    int numProcs = contar_processos(processos);
    int tamPag = gerMem->get_tam_pagina();
    int numFrames = gerMem->get_num_frames();
    int memFisica = numFrames * tamPag;           // recalcula mem física
    Politica pol = gerMem->get_politica();

    // clone
    GerenciadorMemoria clone(memFisica, tamPag, numProcs, memProcs, pol);

    // Ótimo: o clone precisa da linha do tempo futura
    if (pol == Politica::OTIMO)
        clone.set_linha_tempo(linhaTempo, tamLinha);

    faultPorTick.assign(tamLinha, false);

    for (int t = 0; t < tamLinha; t++) {
        int proc = linhaTempo[t][0];
        if (proc == -1) continue;

        int npag = clone.num_paginas(proc);
        int pagina = t % npag;

        ResultadoAcesso r = clone.acessar(proc, pagina, t);

        if (r.page_fault) {
            faultPorTick[t] = true;
            faultsPorProc[proc]++;
        }
    }
}

// preenche a tabela do relatorio
void relatorioWindow::preencherTabela()
{
    int numProcs = contar_processos(processos);
    const char nomes[] = "ABCDEFGHIJ";

    ui->tabelaMetricas->setColumnCount(5);
    ui->tabelaMetricas->setRowCount(numProcs);
    ui->tabelaMetricas->setHorizontalHeaderLabels(
        {tr("Processo"), tr("Chegada"), tr("Resposta"), tr("Espera"), tr("Page Faults")});

    ui->tabelaMetricas->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tabelaMetricas->verticalHeader()->setVisible(false);
    ui->tabelaMetricas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tabelaMetricas->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tabelaMetricas->setAlternatingRowColors(true);

    for (int p = 0; p < numProcs; p++) {
        // acha o tick de conclusão
        int conclusao = 0;
        for (int t = tamLinha - 1; t >= 0; t--) {
            if (linhaTempo[t][0] == p) { conclusao = t + 1; break; }
        }

        int resposta = conclusao - processos[p].tempo_chegada;
        int espera = resposta  - processos[p].tempo_execucao;

        auto mkItem = [](const QString &txt) {
            auto *it = new QTableWidgetItem(txt);
            it->setTextAlignment(Qt::AlignCenter);
            return it;
        };

        ui->tabelaMetricas->setItem(p, 0, mkItem(QString(nomes[p])));
        ui->tabelaMetricas->setItem(p, 1, mkItem(QString::number(processos[p].tempo_chegada)));
        ui->tabelaMetricas->setItem(p, 2, mkItem(QString::number(resposta)));
        ui->tabelaMetricas->setItem(p, 3, mkItem(QString::number(espera)));
        ui->tabelaMetricas->setItem(p, 4, mkItem(QString::number(faultsPorProc[p])));
    }
}

// preenche as labels de resultado
void relatorioWindow::preencherLabels()
{
    // TMR
    float tmt = tempo_medio_turnaround(linhaTempo, tamLinha);
    ui->labelTmr->setText(QString::number(static_cast<double>(tmt), 'f', 2));

    // TME
    float tme = tempo_medio_espera(linhaTempo, tamLinha);
    ui->labelTme->setText(QString::number(static_cast<double>(tme), 'f', 2));

    // total page faults
    ui->labelPageFaults->setText(QString::number(gerMem->get_page_faults()));
}

// gerar outra simulação
void relatorioWindow::on_buttonInicio_clicked()
{
    // reset global
    /*for (int i = 0; i < MAX_PROCESSOS; i++)
        processos[i] = {0, 0, 0, 0};
    QUANTUM_GLOBAL = -1;*/

    if (parentWidget())
        parentWidget()->show();

    QTimer::singleShot(500, this, &relatorioWindow::hide);
}