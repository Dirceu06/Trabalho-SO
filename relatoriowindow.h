#ifndef RELATORIOWINDOW_H
#define RELATORIOWINDOW_H

#include <QMainWindow>

namespace Ui {
class relatorioWindow;
}

class relatorioWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit relatorioWindow(QWidget *parent = nullptr);
    ~relatorioWindow();

private slots:
    void on_buttonInicio_clicked();

private:
    Ui::relatorioWindow *ui;
};

#endif // RELATORIOWINDOW_H
