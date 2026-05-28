#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_selectCsv_clicked();

private:
    bool carregarProcessosCsv(const QString &caminho, QString &erro);
    bool parseLineToProcesso(const QString &linha, Processo &processo, QString &erro);

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
