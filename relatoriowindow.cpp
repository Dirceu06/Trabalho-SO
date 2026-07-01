#include "relatoriowindow.h"
#include "ui_relatoriowindow.h"
#include "Escalonador/auxiliares.h"

#include <QScrollArea>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QFont>
#include <QTimer>
#include <cstdlib>

// paleta compartilhada entre GanttWidget e QueueWidget
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
    QColor(255, 200, 160),  // J pêssego
};

const QColor QueueWidget::CORES_PROC[10] = {
    QColor(100, 180, 255),
    QColor(120, 210, 120),
    QColor(255, 210,  80),
    QColor(220, 130, 220),
    QColor(255, 150,  80),
    QColor( 80, 210, 210),
    QColor(255, 120, 120),
    QColor(160, 200,  80),
    QColor(200, 160, 255),
    QColor(255, 200, 160),
};

// GanttWidget
GanttWidget::GanttWidget(QWidget *parent) : QWidget(parent) {}

void GanttWidget::setDados(int (*lt)[3], int t, int nProcs,
                           const std::vector<bool> &faults)
{
    linhaTempo = lt;
    tam = t;
    numProcs = nProcs;
    faultPorTick = faults;
    int largura = MARGIN_LEFT + tam * CELL_W + 40;
    int altura  = MARGIN_TOP  + numProcs * CELL_H + 44;
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

    // eixo superior
    p.setPen(Qt::black);
    for (int t = 0; t <= tam; t++)
        p.drawText(MARGIN_LEFT + t * CELL_W, MARGIN_TOP - 6, QString::number(t));

    // labels dos processos
    for (int proc = 0; proc < numProcs; proc++)
        p.drawText(6, MARGIN_TOP + proc * CELL_H + CELL_H - 8, QString(nomes[proc]));

    // células
    for (int t = 0; t < tam; t++) {
        int  proc  = linhaTempo[t][0];
        int  x     = MARGIN_LEFT + t * CELL_W;
        bool fault = (t < static_cast<int>(faultPorTick.size())) && faultPorTick[t];

        for (int row = 0; row < numProcs; row++) {
            int y = MARGIN_TOP + row * CELL_H;

            if (proc == row) {
                p.fillRect(x, y, CELL_W - 1, CELL_H - 1, CORES_PROC[row % 10]);

                if (fault) {
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
    for (int t = 0; t <= tam; t++)
        p.drawText(MARGIN_LEFT + t * CELL_W, yBottom + 10, QString::number(t));

    // legenda
    int yLeg = yBottom + 22;
    p.fillRect(MARGIN_LEFT, yLeg, 12, 12, QColor(100, 180, 255));
    p.setPen(QPen(Qt::red, 2));
    p.drawRect(MARGIN_LEFT, yLeg, 12, 12);
    p.setPen(Qt::black);
    p.drawText(MARGIN_LEFT + 16, yLeg + 11, "= Page Fault");
}


//  QueueWidget: Fila de Prontos
QueueWidget::QueueWidget(QWidget *parent) : QWidget(parent) {}

void QueueWidget::setDados(const std::vector<std::vector<int>> &estados,
                           int t, int nProcs)
{
    filaEstados = estados;
    tam = t;
    numProcs = nProcs;

    // altura: número máximo de processos simultâneos na fila em qualquer tick
    int maxFila = 0;
    for (const auto &f : filaEstados)
        if (static_cast<int>(f.size()) > maxFila)
            maxFila = static_cast<int>(f.size());
    if (maxFila == 0) maxFila = 1;

    int largura = MARGIN_LEFT + tam * CELL_W + 40;
    int altura  = MARGIN_TOP  + maxFila * CELL_H + 30;
    setMinimumSize(largura, altura);
    update();
}

void QueueWidget::paintEvent(QPaintEvent *)
{
    if (filaEstados.empty() || tam == 0) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    QFont fontNormal;
    fontNormal.setPointSize(8);
    p.setFont(fontNormal);

    const char nomes[] = "ABCDEFGHIJ";

    // calcula altura real (máx processos em fila em qualquer tick)
    int maxFila = 0;
    for (const auto &f : filaEstados)
        if (static_cast<int>(f.size()) > maxFila)
            maxFila = static_cast<int>(f.size());
    if (maxFila == 0) maxFila = 1;

    // eixo superior (ticks)
    p.setPen(Qt::black);
    for (int t = 0; t <= tam; t++)
        p.drawText(MARGIN_LEFT + t * CELL_W, MARGIN_TOP - 4, QString::number(t));

    // label de posição na fila no eixo esquerdo (1o, 2o, ....)
    for (int row = 0; row < maxFila; row++) {
        QString lbl = QString::number(row + 1) + "º";
        p.drawText(4, MARGIN_TOP + row * CELL_H + CELL_H - 6, lbl);
    }

    // células: para cada tick, empilha os processos da fila de cima para baixo
    for (int t = 0; t < tam; t++) {
        int x = MARGIN_LEFT + t * CELL_W;

        for (int row = 0; row < maxFila; row++) {
            int y = MARGIN_TOP + row * CELL_H;

            if (row < static_cast<int>(filaEstados[t].size())) {
                int proc = filaEstados[t][row];
                // célula colorida com a letra do processo
                p.fillRect(x, y, CELL_W - 1, CELL_H - 1, CORES_PROC[proc % 10]);
                p.setPen(QPen(Qt::darkGray, 1));
                p.drawRect(x, y, CELL_W - 1, CELL_H - 1);

                QFont fBold = fontNormal;
                fBold.setBold(true);
                fBold.setPointSize(9);
                p.setFont(fBold);
                p.setPen(Qt::black);
                // centraliza a letra na célula
                p.drawText(x, y, CELL_W - 1, CELL_H - 1,
                           Qt::AlignCenter, QString(nomes[proc]));
                p.setFont(fontNormal);
            } else {
                // célula vazia
                p.fillRect(x, y, CELL_W - 1, CELL_H - 1, QColor(248, 248, 248));
                p.setPen(QPen(QColor(220, 220, 220), 1));
                p.drawRect(x, y, CELL_W - 1, CELL_H - 1);
            }
        }
    }

    // legenda embaixo
    int yBottom = MARGIN_TOP + maxFila * CELL_H + 6;
    p.setPen(Qt::darkGray);
    p.setFont(fontNormal);
    p.drawText(MARGIN_LEFT, yBottom + 12,
               "Processos aguardando CPU a cada tick (posição 1º = próximo a executar)");
}

//  relatorioWindow
relatorioWindow::relatorioWindow(int (*linha)[3], int tam,
                                 GerenciadorMemoria *gm,
                                 const std::vector<int> &memProcs,
                                 QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::relatorioWindow),
    linhaTempo(linha),
    tamLinha(tam),
    gerMem(gm),
    ganttWidget(nullptr),
    queueWidget(nullptr)
{
    ui->setupUi(this);

    int numProcs = contar_processos(processos);
    faultsPorProc.assign(numProcs, 0);

    coletarFaults(memProcs);
    reconstruirFila();

    // Gantt
    ganttWidget = new GanttWidget();
    ganttWidget->setDados(linhaTempo, tamLinha, numProcs, faultPorTick);

    QScrollArea *scrollGantt = new QScrollArea(ui->centralwidget);
    scrollGantt->setWidget(ganttWidget);
    scrollGantt->setWidgetResizable(false);
    scrollGantt->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollGantt->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollGantt->setGeometry(50, 45, 700, 195);

    // Label "Fila de Prontos"
    QLabel *labelFila = new QLabel("Fila de Prontos (Ready Queue)", ui->centralwidget);
    QFont fLbl;
    fLbl.setPointSize(10);
    fLbl.setBold(true);
    labelFila->setFont(fLbl);
    labelFila->setGeometry(50, 248, 300, 22);

    // QueueWidget
    queueWidget = new QueueWidget();
    queueWidget->setDados(filaEstados, tamLinha, numProcs);

    QScrollArea *scrollQueue = new QScrollArea(ui->centralwidget);
    scrollQueue->setWidget(queueWidget);
    scrollQueue->setWidgetResizable(false);
    scrollQueue->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollQueue->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollQueue->setGeometry(50, 272, 700, 160);

    // Tabela de métricas
    ui->tabelaMetricas->setGeometry(50, 445, 700, 195);

    preencherTabela();
    preencherLabels();
}

relatorioWindow::~relatorioWindow()
{
    delete ui;
    if (linhaTempo) free(linhaTempo);
    delete gerMem;
}


//  reconstruirFila
//  Recria o estado da ready queue em cada tick a partir de linhaTempo[].
//
//  Lógica:
//    - Um processo P entra na fila no tick em que processos[P].tempo_chegada
//      é atingido, ou no tick seguinte à sua preempção/retorno à fila.
//    - P sai da fila quando a CPU começa a executá-lo (linhaTempo[t][0] == P).
//    - Para reconstruir fielmente a ordem FIFO/preempção sem re-executar o
//      escalonador, usamos o próprio linhaTempo como oráculo:
//        * Se linhaTempo[t][0] != linhaTempo[t-1][0] e o processo anterior
//          não terminou, ele voltou para a fila.
//        * Processos recém-chegados (tempo_chegada == t) entram na fila.
//      A ordem dentro da fila a cada tick é a ordem de chegada na fila
//      (FIFO estrita), que é suficiente para visualização.
void relatorioWindow::reconstruirFila()
{
    int numProcs = contar_processos(processos);

    // restante[p] = burst ainda não executado
    std::vector<int>  restante(numProcs);
    std::vector<bool> chegou(numProcs, false);
    for (int i = 0; i < numProcs; i++)
        restante[i] = processos[i].tempo_execucao;

    // fila interna: índices de processo em ordem de entrada
    std::vector<int> fila;
    filaEstados.resize(tamLinha);

    int procAnterior = -1;

    for (int t = 0; t < tamLinha; t++) {
        int procAtual = linhaTempo[t][0];

        // 1) processos que chegam neste tick entram na fila
        for (int i = 0; i < numProcs; i++) {
            if (!chegou[i] && processos[i].tempo_chegada == t) {
                chegou[i] = true;
                // só entra na fila se não for o processo que já vai executar
                if (i != procAtual)
                    fila.push_back(i);
            }
        }

        // 2) processo anterior foi preemptado (trocou sem terminar)?
        if (procAnterior != -1 && procAnterior != procAtual && restante[procAnterior] > 0) {
            fila.push_back(procAnterior);
        }

        // 3) processo atual sai da fila (se estiver nela)
        if (procAtual != -1) {
            fila.erase(std::remove(fila.begin(), fila.end(), procAtual), fila.end());
        }

        // snapshot da fila neste tick (sem o processo em execução)
        filaEstados[t] = fila;

        // 4) atualiza restante do processo executado
        if (procAtual != -1)
            restante[procAtual]--;

        procAnterior = procAtual;
    }
}


//  coletarFaults
void relatorioWindow::coletarFaults(const std::vector<int> &memProcs)
{
    int numProcs   = contar_processos(processos);
    int tamPag     = gerMem->get_tam_pagina();
    int numFrames  = gerMem->get_num_frames();
    int memFisica  = numFrames * tamPag;
    Politica pol   = gerMem->get_politica();

    GerenciadorMemoria clone(memFisica, tamPag, numProcs, memProcs, pol);

    if (pol == Politica::OTIMO)
        clone.set_linha_tempo(linhaTempo, tamLinha);

    faultPorTick.assign(tamLinha, false);

    for (int t = 0; t < tamLinha; t++) {
        int proc = linhaTempo[t][0];
        if (proc == -1) continue;

        int npag    = clone.num_paginas(proc);
        int pagina  = t % npag;

        ResultadoAcesso r = clone.acessar(proc, pagina, t);

        if (r.page_fault) {
            faultPorTick[t]    = true;
            faultsPorProc[proc]++;
        }
    }
}


//  preencherTabela
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
        int conclusao = 0;
        for (int t = tamLinha - 1; t >= 0; t--) {
            if (linhaTempo[t][0] == p) { conclusao = t + 1; break; }
        }

        int resposta = conclusao - processos[p].tempo_chegada;
        int espera = resposta - processos[p].tempo_execucao;

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


//  preencherLabels
void relatorioWindow::preencherLabels()
{
    float tmt = tempo_medio_turnaround(linhaTempo, tamLinha);
    ui->labelTmr->setText(QString::number(static_cast<double>(tmt), 'f', 2));

    float tme = tempo_medio_espera(linhaTempo, tamLinha);
    ui->labelTme->setText(QString::number(static_cast<double>(tme), 'f', 2));

    ui->labelPageFaults->setText(QString::number(gerMem->get_page_faults()));
}


//  Slot: voltar para tela inicial
void relatorioWindow::on_buttonInicio_clicked()
{
    if (parentWidget())
        parentWidget()->show();

    QTimer::singleShot(500, this, &relatorioWindow::hide);
}