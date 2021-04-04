#include "mainwindow.h"
#include "coeffmodel.h"
#include "datamodel.h"
#include "polynomial.h"
#include "ui_mainwindow.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QSettings>
#include <QTextStream>
#include <charconv>

auto dataModel = (DataModel*)nullptr;
auto coeffModel = (CoeffModel*)nullptr;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //
{
    ui->setupUi(this);

    /*auto*/ dataModel = new DataModel(ui->tableView);
    ui->tableView->setModel(dataModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->hideColumn(DataModel::ColumnComp);
    ui->gvData->setData(dataModel->data(), 0);

    /*auto*/ coeffModel = new CoeffModel(ui->tableView_2);
    ui->tableView_2->setModel(coeffModel);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_2->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(this, &MainWindow::dataLoaded, dataModel, &DataModel::setNewData);
    connect(this, &MainWindow::dataLoaded, [this](const Data& data) { ui->gvData->setData(data); });

    connect(dataModel, &DataModel::dataChanged_, [this](const Data& data) { ui->gvData->setData(data); });
    connect(dataModel, &DataModel::dataChanged_, [this](const Data&) { emit ui->sbxDeg->valueChanged(ui->sbxDeg->value()); });

    connect(coeffModel, &CoeffModel::dataChanged_, &poly, &Polynomial::setDegrees);
    //    connect(coeffModel, &DataModel::dataChanged_, [this](const Degrees&) { emit ui->sbxDeg->valueChanged(ui->sbxDeg->value()); });

    { // menu File
        auto menu = menuBar()->addMenu("&Файл");

        menu->addAction(
            QIcon::fromTheme("document-open"), "Загрузить данные...", [this] {
                dataFileName = QFileDialog::getOpenFileName(this, "Загрузить данные...", dataFileName, "*.txt;;*.bin");
                dataModel->load(dataFileName);
            },
            QKeySequence::Open);

        menu->addAction(
            QIcon::fromTheme("document-save"), "Сохранить данные",
            [this] {
                if (dataFileName.isEmpty())
                    dataFileName = QFileDialog::getSaveFileName(this, "Сохранить данные", "", "*.txt;;*.bin");
                dataModel->save(dataFileName);
            });

        menu->addAction(
            QIcon::fromTheme("document-save-as"), "Сохранить данные как...",
            [this] {
                dataFileName = QFileDialog::getSaveFileName(this, "Сохранить данные как...", dataFileName, "*.txt;;*.bin");
                dataModel->save(dataFileName);
            });

        menu->addSeparator();

        menu->addAction(
            QIcon::fromTheme("document-open"), "Загрузить коэффициенты...",
            [this] {
                coefFileName = QFileDialog::getOpenFileName(this, "Загрузить коэффициенты...", coefFileName, "*.txt;;*.bin");
                coeffModel->load(coefFileName);
            });

        menu->addAction(
            QIcon::fromTheme("document-save"), "Сохранить коэффициенты",
            [this] {
                if (coefFileName.isEmpty())
                    coefFileName = QFileDialog::getSaveFileName(this, "Сохранить коэффициенты", "", "*.txt;;*.bin");
                coeffModel->save(coefFileName);
            },
            QKeySequence::Save);

        menu->addAction(
            QIcon::fromTheme("document-save-as"), "Сохранить коэффициенты как...",
            [this] {
                coefFileName = QFileDialog::getSaveFileName(this, "Сохранить коэффициенты как...", coefFileName, "*.txt;;*.bin");
                coeffModel->save(coefFileName);
            },
            QKeySequence("Ctrl+Alt+S"));

        menu->addSeparator();

        menu->addAction(QIcon::fromTheme("application-exit"), "Выход", this, &MainWindow::close, QKeySequence("Ctrl+Q"));
    }
    { // menu Справка
        auto menu = menuBar()->addMenu("Справка");
        menu->addAction(QIcon::fromTheme("QtProject-designer"), "О Qt", [] { qApp->aboutQt(); });
    }

    connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), coeffModel, &CoeffModel::setPrecision);
    connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), dataModel, &DataModel::setPrecision);
    connect(ui->sbxDeg, qOverload<int>(&QSpinBox::valueChanged), [/*dataModel, coeffModel,*/ this](int deg) { //
        poly.setData(dataModel->data());
        poly.calcDegrees(deg);
        coeffModel->setCoeffData(poly.degrees());
        {
            auto data1 = poly.calcData(dataModel->data());
            ui->gvData->setData(data1, 1);
            auto data2 = dataModel->data();
            for (int i = 0; i < data1.size(); ++i)
                data1[i].ry() -= data2[i].y();
            ui->gvDelta->setDeltaData(data1);
        }
    });

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

    settings.setValue("coefFileName", coefFileName);
    settings.setValue("dataFileName", dataFileName);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.endGroup();
}

void MainWindow::loadSetings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    coefFileName = settings.value("coefFileName").toString();
    dataFileName = settings.value("dataFileName").toString();

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();

    static_cast<DataModel*>(ui->tableView->model())->load(dataFileName);
}
