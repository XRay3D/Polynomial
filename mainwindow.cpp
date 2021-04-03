#include "mainwindow.h"
#include "coeffmodel.h"
#include "datamodel.h"
#include "polynomial.h"
#include "ui_mainwindow.h"

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

    ui->graphicsView->setData(model->data());

    constexpr size_t N = 16;
    Polynomial<N> poly;
    poly.setData(model->data());
    for (int var = 0; var < 1000; ++var) {
        poly.calcCoef();
    }

    auto model2 = new CoeffModel(ui->tableView_2);
    model2->setData(poly.getCoeff());

    ui->tableView_2->setModel(model2);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_2->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    auto data1 = poly.calc(model->data());
    ui->graphicsView->setData3(data1);

    auto data2 = model->data();
    for (int i = 0; i < data1.size(); ++i)
        data1[i].ry() -= data2[i].y();
    ui->graphicsView_3->setData2(data1);

    loadSetings();
    //    QTimer::singleShot(100, this, &QWidget::close);
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
    settings.setValue("avg", Timer::avg / Timer::ctr);

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
