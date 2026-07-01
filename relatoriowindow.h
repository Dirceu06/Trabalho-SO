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

// Gantt
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
    std::vector<bool> faultPorTick;

    static const QColor CORES_PROC[10];
    static const int    CELL_W      = 32;
    static const int    CELL_H      = 30;
    static const int    MARGIN_LEFT = 50;
    static const int    MARGIN_TOP  = 24;
};

// Fila de Prontos
// filaEstados[t] = lista de índices de processo na fila de prontos no tick t
//   (processo em execução nao entra na lista, só os que aguardam)
class QueueWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QueueWidget(QWidget *parent = nullptr);

    void setDados(const std::vector<std::vector<int>> &filaEstados,
                  int tam, int numProcs);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<std::vector<int>> filaEstados;
    int tam      = 0;
    int numProcs = 0;

    static const QColor CORES_PROC[10];
    static const int    CELL_W      = 32;
    static const int    CELL_H      = 24;
    static const int    MARGIN_LEFT = 50;
    static const int    MARGIN_TOP  = 20;
    static const int    MAX_FILA    = 10;
};

// Janela de Relatório
class relatorioWindow : public QMainWindow
{
    Q_OBJECT

public:
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

    std::vector<bool>  faultPorTick;
    std::vector<int>   faultsPorProc;

    GanttWidget  *ganttWidget;
    QueueWidget  *queueWidget;

    std::vector<std::vector<int>> filaEstados; // [tick] -> {proc0, proc1, ...}

    void coletarFaults(const std::vector<int> &memProcs);
    void reconstruirFila();
    void preencherTabela();
    void preencherLabels();
};

#endif // RELATORIOWINDOW_H