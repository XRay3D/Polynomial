#include "mainwindow.h"

#include <QApplication>
#include <qglobal.h>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    a.setOrganizationName("XrSoft");
    a.setApplicationName("Polynomial");

    MainWindow w;
    w.show();
    return a.exec();
}
