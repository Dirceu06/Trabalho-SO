#include "relatoriowindow.h"
#include "ui_relatoriowindow.h"

relatorioWindow::relatorioWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::relatorioWindow)
{
    ui->setupUi(this);
}

relatorioWindow::~relatorioWindow()
{
    delete ui;
}

void relatorioWindow::on_buttonInicio_clicked()
{
    // botao voltar pra mainWindow
    // limpar variáveis globais
}

