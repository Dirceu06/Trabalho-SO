#include "escalonador.h"
#include "ui_escalonador.h"
#include "Escalonador/auxiliares.h"

#include <QIntValidator>
#include <QMessageBox>
#include <QPushButton>

static const char *selectedButtonStyle = "background-color: #0078d7; color: white;";
static const char *normalButtonStyle = "";

escalonador::escalonador(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::escalonador)
    , selectedScheduler(SchedulerType::None)
{
    ui->setupUi(this);
    ui->buttonRr->setCheckable(true);
    ui->buttonSjf->setCheckable(true);
    ui->buttonPriori->setCheckable(true);
    ui->lineQuantum->setValidator(new QIntValidator(0, 2147483647, this));
    ui->lineQuantum->setVisible(false);
    ui->labelQuantum->setVisible(false);
    updateButtonHighlight();
}

escalonador::~escalonador()
{
    delete ui;
}

void escalonador::selectScheduler(SchedulerType scheduler)
{
    selectedScheduler = scheduler;
    const bool isRoundRobin = (scheduler == SchedulerType::RoundRobin);
    ui->lineQuantum->setVisible(isRoundRobin);
    ui->labelQuantum->setVisible(isRoundRobin);
    updateButtonHighlight();
}

void escalonador::updateButtonHighlight()
{
    ui->buttonRr->setStyleSheet(selectedScheduler == SchedulerType::RoundRobin ? selectedButtonStyle : normalButtonStyle);
    ui->buttonSjf->setStyleSheet(selectedScheduler == SchedulerType::SJF ? selectedButtonStyle : normalButtonStyle);
    ui->buttonPriori->setStyleSheet(selectedScheduler == SchedulerType::Prioridade ? selectedButtonStyle : normalButtonStyle);

    ui->buttonRr->setChecked(selectedScheduler == SchedulerType::RoundRobin);
    ui->buttonSjf->setChecked(selectedScheduler == SchedulerType::SJF);
    ui->buttonPriori->setChecked(selectedScheduler == SchedulerType::Prioridade);
}

void escalonador::on_buttonRr_clicked()
{
    selectScheduler(SchedulerType::RoundRobin);
}


void escalonador::on_buttonSjf_clicked()
{
    selectScheduler(SchedulerType::SJF);
}


void escalonador::on_buttonPriori_clicked()
{
    selectScheduler(SchedulerType::Prioridade);
}


void escalonador::on_lineQuantum_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        QUANTUM_GLOBAL = -1;
        return;
    }

    bool ok = false;
    int valor = arg1.toInt(&ok);
    if (!ok || valor < 0) {
        QUANTUM_GLOBAL = -1;
        return;
    }

    QUANTUM_GLOBAL = valor;
}

void escalonador::on_buttonGerarRelatorio_clicked()
{
    if (selectedScheduler == SchedulerType::None) {
        QMessageBox::warning(this, tr("Escalonador não selecionado"), tr("Selecione um escalonador antes de gerar o relatório."));
        return;
    }

    if (selectedScheduler == SchedulerType::RoundRobin && QUANTUM_GLOBAL < 0) {
        QMessageBox::warning(this, tr("Quantum inválido"), tr("Informe um quantum válido (inteiro não negativo) para Round Robin."));
        return;
    }

    int tam = 0;
    int (*linha)[3] = nullptr;

    switch (selectedScheduler) {
    case SchedulerType::RoundRobin:
        linha = rr_linha(QUANTUM_GLOBAL, &tam);
        printf("=== Round Robin  (quantum = %d) ===\n", QUANTUM_GLOBAL);
        break;

    case SchedulerType::SJF:
        linha = sjf_linha(&tam);
        break;

    case SchedulerType::Prioridade:
        linha = priori_linha(&tam);
        break;

    default:
        return;
    }

    if (linha) {
        imprimir_gantt(linha, tam);
        imprimir_metricas(linha, tam);
        free(linha);
    }
}

