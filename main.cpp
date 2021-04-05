#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QIcon::setThemeName("breeze");

    app.setOrganizationName("XrSoft");
    app.setApplicationName("Polynomial");

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, app.applicationDirPath());

    MainWindow w;
    w.show();
    return app.exec();
}
