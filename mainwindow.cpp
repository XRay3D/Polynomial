#include "mainwindow.h"
#include "coeffmodel.h"
#include "datamodel.h"
#include "polynomial.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //
{
    ui->setupUi(this);

    auto dataModel = new DataModel(ui->tableView);
    ui->tableView->setModel(dataModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->gvData->setData(dataModel->data(), 0);

    auto coeffModel = new CoeffModel(ui->tableView_2);
    ui->tableView_2->setModel(coeffModel);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_2->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    { // menu File
        auto menu = menuBar()->addMenu("&Файл");

        menu->addAction(
            QIcon::fromTheme("document-open"), "Открыть...",
            [this, dataModel] { dataFileName = QFileDialog::getOpenFileName(this, "Open", dataFileName, "Tests (*.txt)"); dataModel->load(dataFileName); },
            QKeySequence::Open);

        menu->addSeparator();

        menu->addAction(
            QIcon::fromTheme("document-save"), "Сохранить",
            [this, coeffModel] { if(dataFileName.isEmpty()) coefFileName = QFileDialog::getSaveFileName(this, "Save", "", "Tests (*.txt)"); coeffModel->save(coefFileName); },
            QKeySequence::Save);

        menu->addAction(
            QIcon::fromTheme("document-save-as"), "Сохранить как...",
            [this, coeffModel] { coefFileName = QFileDialog::getSaveFileName(this, "Save As", coefFileName, "Tests (*.txt)"); coeffModel->save(coefFileName); },
            QKeySequence::SaveAs);

        menu->addSeparator();
        menu->addAction(QIcon::fromTheme("application-exit"), "Выход", this, &MainWindow::close, QKeySequence("Ctrl+Q"));
    }
    { // menu Справка
        auto menu = menuBar()->addMenu("Справка");
        menu->addAction(QIcon::fromTheme("QtProject-designer"), "О Qt", [] { qApp->aboutQt(); });
    }

    connect(ui->sbxPrec, qOverload<int>(&QSpinBox::valueChanged), coeffModel, &CoeffModel::setPrec);
    connect(ui->sbxDeg, qOverload<int>(&QSpinBox::valueChanged), [dataModel, coeffModel, this](int deg) { //
        poly.setData(dataModel->data());
        poly.calcCoef(deg);
        coeffModel->setData(poly.getCoeff());
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
