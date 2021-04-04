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

signals:
    void dataLoaded(const Data&);
    void coeffLoaded(const Degrees&);

private:
    Ui::MainWindow* ui;

    void saveSetings();
    void loadSetings();

    QString coefFileName;
    QString dataFileName;

    Polynomial poly;
};
#endif // MAINWINDOW_H
