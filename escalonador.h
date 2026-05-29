#ifndef ESCALONADOR_H
#define ESCALONADOR_H

#include <QMainWindow>

namespace Ui {
class escalonador;
}

class escalonador : public QMainWindow
{
    Q_OBJECT

public:
    explicit escalonador(QWidget *parent = nullptr);
    ~escalonador();

private slots:
    void on_buttonRr_clicked();
    void on_buttonSjf_clicked();
    void on_buttonPriori_clicked();
    void on_buttonFifo_clicked();
    void on_buttonLru_clicked();
    void on_buttonOtimo_clicked();
    void on_lineQuantum_textChanged(const QString &arg1);
    void on_lineMemFisica_textChanged(const QString &arg1);
    void on_lineTamPagina_textChanged(const QString &arg1);
    void on_buttonGerarRelatorio_clicked();

private:
    enum class SchedulerType {
        None,
        RoundRobin,
        SJF,
        Prioridade
    };

    enum class MemoryPolicy {
        None,
        FIFO,
        LRU,
        Otimo
    };

    void selectScheduler(SchedulerType scheduler);
    void selectMemoryPolicy(MemoryPolicy policy);
    void updateButtonHighlight();

    Ui::escalonador *ui;
    SchedulerType selectedScheduler;
    MemoryPolicy selectedMemoryPolicy;
    int memoriaFisicaMB;
    int tamPaginaMB;
};

#endif // ESCALONADOR_H
