#ifndef RELATORIOWINDOW_H
#define RELATORIOWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <vector>
#include "Memoria/GerenciadorMemoria.h"
#include "Escalonador/base.h"

namespace Ui {
    class relatorioWindow;
}

class GanttWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GanttWidget(QWidget *parent = nullptr);

    void setDados(int (*lt)[3], int tam, int numProcs,
                  const std::vector<bool> &faultPorTick);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int  (*linhaTempo)[3] = nullptr;
    int  tam              = 0;
    int  numProcs         = 0;
    std::vector<bool> faultPorTick; // faultPorTick[t] = true se teve PF no tick t

    static const QColor CORES_PROC[10];
    static const int    CELL_W      = 32;
    static const int    CELL_H      = 30;
    static const int    MARGIN_LEFT = 50;
    static const int    MARGIN_TOP  = 24;
};

class relatorioWindow : public QMainWindow
{
    Q_OBJECT

public:
    // linha: linha do tempo gerada pelo escalonador
    // tam: tamanho da linha do tempo
    // gm: GerenciadorMemoria
    // memProcs: vetor com memoria de cada processo em MB (pro clone)
    explicit relatorioWindow(int (*linha)[3], int tam,
                             GerenciadorMemoria *gm,
                             const std::vector<int> &memProcs,
                             QWidget *parent = nullptr);
    ~relatorioWindow();

private slots:
    void on_buttonInicio_clicked();

private:
    Ui::relatorioWindow *ui;

    int  (*linhaTempo)[3];
    int  tamLinha;
    GerenciadorMemoria *gerMem;

    // faultPorTick[t] = true se houve page fault no tick t
    std::vector<bool>  faultPorTick;
    // faultsPorProc[p] = total de page faults do processo p
    std::vector<int>   faultsPorProc;

    GanttWidget *ganttWidget;

    // roda um clone do GerenciadorMemoria pra coletar faults por tick/proc
    void coletarFaults(const std::vector<int> &memProcs);

    void preencherTabela();
    void preencherLabels();
};

#endif // RELATORIOWINDOW_H