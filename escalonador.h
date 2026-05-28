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
    void on_lineQuantum_textChanged(const QString &arg1);
    void on_buttonGerarRelatorio_clicked();

private:
    enum class SchedulerType {
        None,
        RoundRobin,
        SJF,
        Prioridade
    };

    void selectScheduler(SchedulerType scheduler);
    void updateButtonHighlight();

    Ui::escalonador *ui;
    SchedulerType selectedScheduler;
};

#endif // ESCALONADOR_H
