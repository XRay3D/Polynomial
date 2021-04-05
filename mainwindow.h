#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "polynomial.h"

#include <QMainWindow>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct Bench : public QThread {

    Polynomial& poly;
    Bench(Polynomial& poly)
        : poly{poly} { }
    void run() override {
        for(int i = 0; i < 100; ++i) {
            emit val(i);
            if(isInterruptionRequested())
                return;
            for(int d = 1; d <= MaxDegree; ++d) {
                poly.calcDegrees(d);
            }
        }
    }
    Q_OBJECT
signals:
    void val(int);
};

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

    void bench();
};
#endif // MAINWINDOW_H
