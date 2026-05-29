#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "escalonador.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::parseLineToProcesso(const QString &linha, Processo &processo, QString &erro)
{
    QStringList partes = linha.split(',', Qt::SkipEmptyParts);
    if (partes.size() != 4) {
        erro = QStringLiteral("Cada linha deve conter 4 valores separados por vírgula.");
        return false;
    }

    bool ok = false;
    processo.tempo_chegada = partes[0].trimmed().toInt(&ok);
    if (!ok) { erro = QStringLiteral("Tempo de chegada inválido."); return false; }

    processo.tempo_execucao = partes[1].trimmed().toInt(&ok);
    if (!ok) { erro = QStringLiteral("Tempo de execução inválido."); return false; }

    processo.prioridade = partes[2].trimmed().toInt(&ok);
    if (!ok) { erro = QStringLiteral("Prioridade inválida."); return false; }

    processo.memoria = partes[3].trimmed().toInt(&ok);
    if (!ok) { erro = QStringLiteral("Memória inválida."); return false; }

    if (processo.tempo_execucao <= 0) {
        erro = QStringLiteral("Tempo de execução deve ser maior que zero.");
        return false;
    }

    return true;
}

bool MainWindow::carregarProcessosCsv(const QString &caminho, QString &erro)
{
    QFile arquivo(caminho);
    if (!arquivo.open(QFile::ReadOnly | QFile::Text)) {
        erro = QStringLiteral("Não foi possível abrir o arquivo.");
        return false;
    }

    QString texto = arquivo.readAll();
    arquivo.close();

    QStringList linhas = texto.split('\n', Qt::SkipEmptyParts);
    if (linhas.isEmpty()) {
        erro = QStringLiteral("O arquivo CSV está vazio.");
        return false;
    }

    int linhaIndex = 0;
    for (int i = 0; i < MAX_PROCESSOS; ++i)
        processos[i] = {0, 0, 0, 0};

    for (const QString &linhaBruta : linhas) {
        QString linha = linhaBruta.trimmed();
        if (linha.isEmpty())
            continue;

        if (linhaIndex >= MAX_PROCESSOS) {
            erro = QStringLiteral("O arquivo CSV contém mais de %1 processos.").arg(MAX_PROCESSOS);
            return false;
        }

        Processo processo;
        if (!parseLineToProcesso(linha, processo, erro))
            return false;

        processos[linhaIndex++] = processo;
    }

    if (linhaIndex == 0) {
        erro = QStringLiteral("O arquivo CSV não contém processos válidos.");
        return false;
    }

    return true;
}

void MainWindow::on_selectCsv_clicked()
{
    QString caminho = QFileDialog::getOpenFileName(
        this,
        tr("Selecionar arquivo CSV"),
        QString(),
        tr("Arquivos CSV (*.csv);;Todos os arquivos (*)")
    );

    if (caminho.isEmpty())
        return;

    QString erro;
    if (!carregarProcessosCsv(caminho, erro)) {
        QMessageBox::warning(this, tr("Arquivo inválido"), tr("Arquivo CSV inválido:\n%1").arg(erro));
        return;
    }

    QMessageBox::information(this, tr("Arquivo carregado"), tr("Arquivo CSV válido carregado com sucesso."));

    escalonador *janelaEscalonador = new escalonador(this);
    janelaEscalonador->setAttribute(Qt::WA_DeleteOnClose);
    connect(janelaEscalonador, &QObject::destroyed, qApp, &QCoreApplication::quit);
    connect(janelaEscalonador, &QObject::destroyed, this, &MainWindow::close);
    janelaEscalonador->show();

    QTimer::singleShot(500, this, &MainWindow::hide);
}