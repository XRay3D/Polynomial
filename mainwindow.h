#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "polynomial.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;

    void saveSetings();
    void loadSetings();

    QString degreesFileName;
    QString dataFileName;

    Polynomial poly;

    void loadData();
    void saveData();
    void saveDataAs();

    void loadDegrees();
    void saveDegrees();
    void saveDegreesAs();

    QString actionName();
};
#endif // MAINWINDOW_H
