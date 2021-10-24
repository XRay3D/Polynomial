#include "mainwindow.h"
#include "datamodel.h"
#include "degreesmodel.h"
#include "polynomial.h"
#include "ui_mainwindow.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSettings>
#include <QTextStream>
#include <QThread>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) //
{
    ui->setupUi(this);

    ui->gvData->init({ "Данные", "Пересчет" });
    ui->gvDelta->init({ "Oшибка" });

    connect(ui->chbxPrec, &QCheckBox::toggled, &poly, &Polynomial::setUsePrecision);
    connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), &poly, &Polynomial::setPrecision);

    auto dataModel = new DataModel(ui->tvData);
    ui->tvData->setModel(dataModel);

    [this]<auto... D>(Seq<D...>) {
        (ui->cbxDegrees->addItem(QString::number(D + 1), D + 1), ...);
        QSize size { 0, QFontMetrics(font()).height() + 6 };
        (ui->cbxDegrees->setItemData(D, size, Qt::SizeHintRole), ...);
        (ui->cbxDegrees->setItemData(D, Qt::AlignCenter, Qt::TextAlignmentRole), ...);
        ui->cbxDegrees->setMaxVisibleItems(sizeof...(D));
    }
    (MakeSeq<MaxDegree> {});

    auto degreesModel = new DegreesModel(ui->tvDegrees);
    ui->tvDegrees->setModel(degreesModel);

    { // menu File
        auto menu = menuBar()->addMenu("&Файл");

        menu->addAction(QIcon::fromTheme("document-open"), "Загрузить данные...",
            this, &MainWindow::loadData, QKeySequence::Open);

        menu->addAction(QIcon::fromTheme("document-save"), "Сохранить данные",
            this, &MainWindow::saveData);

        menu->addAction(QIcon::fromTheme("document-save-as"), "Сохранить данные как...",
            this, &MainWindow::saveDataAs);

        menu->addSeparator();

        menu->addAction(QIcon::fromTheme("document-open"), "Загрузить коэффициенты...",
            this, &MainWindow::loadDegrees);

        menu->addAction(QIcon::fromTheme("document-save"), "Сохранить коэффициенты",
            this, &MainWindow::saveDegrees, QKeySequence::Save);

        menu->addAction(QIcon::fromTheme("document-save-as"), "Сохранить коэффициенты как...",
            this, &MainWindow::saveDegreesAs, QKeySequence("Ctrl+Alt+S"));

        menu->addSeparator();

        menu->addAction(QIcon::fromTheme("application-exit"), "Выход", this, &MainWindow::close, QKeySequence("Ctrl+Q"));
    }
    { // menu Справка
        auto menu = menuBar()->addMenu("Справка");
        menu->addAction(QIcon::fromTheme("help-about"), "О Qt", [] { qApp->aboutQt(); });
    }

    { // toolbar Управление

        auto toolbar = addToolBar("Управление");
        toolbar->setIconSize({ 22, 22 });
        auto action1 = toolbar->addAction(QIcon::fromTheme("format-precision-less"), "", [this] { ui->sbxPrecision->setValue(ui->sbxPrecision->value() - 1); });
        auto action2 = toolbar->addAction(QIcon::fromTheme("format-precision-more"), "", [this] { ui->sbxPrecision->setValue(ui->sbxPrecision->value() + 1); });
        connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), [this, action1, action2]() {
            action1->setEnabled(ui->sbxPrecision->value() != ui->sbxPrecision->minimum());
            action2->setEnabled(ui->sbxPrecision->value() != ui->sbxPrecision->maximum());
        });
        toolbar->addSeparator();
        action1 = toolbar->addAction(QIcon::fromTheme("labplot-xy-fit-curve"), "Рассчитать полином", [this] {
            poly.calcDegrees(ui->cbxDegrees->currentData().toInt());
        });

        connect(ui->chbxPrec, &QCheckBox::toggled, action1, &QAction::trigger);
        connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), action1, &QAction::trigger);

        action1 = toolbar->addAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), "Пересчитать ошибку", [this, dataModel] {
            poly.calcDataAndDeltaErr(dataModel->data());
        });

        connect(ui->chbxPrec, &QCheckBox::toggled, action1, &QAction::trigger);
        connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), action1, &QAction::trigger);

        toolbar->addSeparator();
        toolbar->addAction(QIcon::fromTheme("help-about"), "Бенчмарк", this, &MainWindow::bench);
    }

    connect(&poly, &Polynomial::dataChanged, ui->gvData, &ChartView::setData1);
    connect(&poly, &Polynomial::deltaChanged, ui->gvDelta, &ChartView::setDeltaData);
    connect(&poly, &Polynomial::degreesChanged, degreesModel, &DegreesModel::setCoeffData);
    connect(&poly, &Polynomial::dataChanged, dataModel, &DataModel::setNewData);

    connect(degreesModel, &DegreesModel::dataChanged_, &poly, &Polynomial::setDegrees);

    connect(dataModel, &DataModel::dataChanged_, &poly, &Polynomial::setData);
    connect(dataModel, &DataModel::dataChanged_, [this](const Data&) { emit ui->cbxDegrees->currentIndexChanged(0); });
    connect(dataModel, &DataModel::dataChanged_, ui->gvData, &ChartView::setData0);

    connect(ui->cbxDegrees, qOverload<int>(&QComboBox::currentIndexChanged), [this] { poly.calcDegrees(ui->cbxDegrees->currentData().toInt()); });
    connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), degreesModel, &DegreesModel::setPrecision);
    connect(ui->sbxPrecision, qOverload<int>(&QSpinBox::valueChanged), dataModel, &DataModel::setPrecision);

    loadSetings();
}

MainWindow::~MainWindow() {
    saveSetings();
    delete ui;
}

void MainWindow::saveSetings() {
    QSettings settings;
    settings.beginGroup("MainWindow");

    settings.setValue("degreesFileName", degreesFileName);
    settings.setValue("dataFileName", dataFileName);

    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("cbxDegrees", ui->cbxDegrees->currentIndex());
    settings.setValue("sbxPrecision", ui->sbxPrecision->value());
    settings.setValue("splitter", ui->splitter->saveState());
    settings.endGroup();
}

void MainWindow::loadSetings() {
    QSettings settings;
    settings.beginGroup("MainWindow");

    degreesFileName = settings.value("degreesFileName").toString();
    dataFileName = settings.value("dataFileName").toString();

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    ui->cbxDegrees->setCurrentIndex(settings.value("cbxDegrees").toInt());
    ui->sbxPrecision->setValue(settings.value("sbxPrecision").toInt());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    settings.endGroup();

    ui->tvData->model<DataModel>()->load(dataFileName);
}

void MainWindow::loadData() {
    if (auto fileName { QFileDialog::getOpenFileName(this, actionName(), dataFileName, "*.txt;;*.bin") }; fileName.isEmpty())
        return;
    else
        dataFileName = fileName;
    ui->tvData->model<DataModel>()->load(dataFileName);
}

void MainWindow::saveData() {
    if (dataFileName.isEmpty())
        dataFileName = QFileDialog::getSaveFileName(this, actionName(), "", "*.txt;;*.bin");
    ui->tvData->model<DataModel>()->save(dataFileName);
}

void MainWindow::saveDataAs() {
    dataFileName = QFileDialog::getSaveFileName(this, actionName(), dataFileName, "*.txt;;*.bin");
    ui->tvData->model<DataModel>()->save(dataFileName);
}

void MainWindow::loadDegrees() {
    if (auto fileName { QFileDialog::getOpenFileName(this, actionName(), degreesFileName, "*.txt;;*.bin") }; fileName.isEmpty())
        return;
    else
        degreesFileName = fileName;
    ui->tvData->model<DegreesModel>()->load(degreesFileName);
}

void MainWindow::saveDegrees() {
    if (degreesFileName.isEmpty())
        degreesFileName = QFileDialog::getSaveFileName(this, actionName(), "", "*.txt;;*.bin");
    ui->tvData->model<DegreesModel>()->save(degreesFileName);
}

void MainWindow::saveDegreesAs() {
    degreesFileName = QFileDialog::getSaveFileName(this, actionName(), degreesFileName, "*.txt;;*.bin");
    ui->tvData->model<DegreesModel>()->save(degreesFileName);
}

QString MainWindow::actionName() {
    if (auto action = qobject_cast<QAction*>(sender()); action)
        return action->text();
    return {};
}

void MainWindow::bench() {
    Timer::avg.clear();
    Timer::ctr.clear();
    QProgressDialog pd;
    pd.setMaximum(99);
    Polynomial poly;
    poly.setData(ui->tvData->model<DataModel>()->data());
    Bench run(poly);
    connect(&pd, &QProgressDialog::canceled, &run, &QThread::requestInterruption);
    connect(&run, &Bench::val, &pd, &QProgressDialog::setValue);
    run.start();
    pd.exec();
    run.wait();
    class Dialog : public QDialog {
    public:
        Dialog() {
            if (objectName().isEmpty())
                setObjectName(QString::fromUtf8("Dialog"));
            resize(400, 720);
            setWindowTitle("Бенчмарк");

            auto verticalLayout = new QVBoxLayout(this);
            verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
            verticalLayout->setContentsMargins(6, 6, 6, 6);

            auto tableView = new QTableView(this);
            tableView->setObjectName(QString::fromUtf8("tableView"));
            verticalLayout->addWidget(tableView);

            struct Model : public QAbstractTableModel {
                std::vector<std::pair<const char*, double>> m_data;

            public:
                Model() {
                    for (auto [key, val] : Timer::avg)
                        m_data.emplace_back(key, val / Timer::ctr[key]);
                    std::ranges::reverse(m_data);
                }
                int rowCount(const QModelIndex& = {}) const override { return m_data.size(); }
                int columnCount(const QModelIndex& = {}) const override { return 1; }
                QVariant data(const QModelIndex& index, int role) const override {
                    if (role == Qt::DisplayRole)
                        return m_data[index.row()].second;
                    else if (role == Qt::TextAlignmentRole)
                        return Qt::AlignCenter;
                    return {};
                }
                QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
                    if (role == Qt::DisplayRole) {
                        if (orientation == Qt::Horizontal)
                            return "Время, мксек.";
                        else
                            return m_data[section].first;
                    }
                    return {};
                }
                Qt::ItemFlags flags(const QModelIndex& = {}) const override { return Qt::ItemIsEnabled; }
            };
            tableView->setModel(new Model);
            tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            QMetaObject::connectSlotsByName(this);
        }
    } dialog;
    dialog.exec();
}
