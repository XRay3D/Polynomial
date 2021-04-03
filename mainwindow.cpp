#include "mainwindow.h"
#include "coeffmodel.h"
#include "datamodel.h"
#include "polynomial.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //
{
    ui->setupUi(this);

    auto model = new DataModel(ui->tableView);
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->gvData->setData(model->data(), 0);

    auto model2 = new CoeffModel(ui->tableView_2);
    ui->tableView_2->setModel(model2);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_2->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(ui->sbxPrec, qOverload<int>(&QSpinBox::valueChanged), model2, &CoeffModel::setPrec);
    connect(ui->sbxDeg, qOverload<int>(&QSpinBox::valueChanged), [model, model2, this](int deg) { //
        Polynomial poly;
        poly.setData(model->data());
        poly.calcCoefCt(deg);
        model2->setData(poly.getCoeff());
        {
            auto data1 = poly.calcData(model->data());
            ui->gvData->setData(data1, 1);
            auto data2 = model->data();
            for (int i = 0; i < data1.size(); ++i)
                data1[i].ry() -= data2[i].y();
            ui->gvDelta->setDeltaData(data1, 1);
        }
        poly.calcCoefRt(deg);
        model2->setData(poly.getCoeff(), 1);
        {
            auto data1 = poly.calcData(model->data());
            ui->gvData->setData(data1, 2);
            auto data2 = model->data();
            for (int i = 0; i < data1.size(); ++i)
                data1[i].ry() -= data2[i].y();
            ui->gvDelta->setDeltaData(data1, 2);
        }
    });

    for (int var = 0; var < 100; ++var) {
        ui->sbxDeg->valueChanged(var % 32);
    }

    loadSetings();
}

MainWindow::~MainWindow()
{
    saveSetings();
    delete ui;
}

void MainWindow::saveSetings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    qDebug() << "last avg" << settings.value("avg").toDouble() << "us";
    settings.setValue("avg", TimerCt::avg / TimerCt::ctr);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.endGroup();
}

void MainWindow::loadSetings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();
}
