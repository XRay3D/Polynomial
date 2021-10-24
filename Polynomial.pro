QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DESTDIR = $$_PRO_FILE_PWD_/bin

#CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += \
    QT_DISABLE_DEPRECATED_BEFORE=0x060000 \   # disables all the APIs deprecated before Qt 6.0.0
    _USE_MATH_DEFINES

msvc:{
    QMAKE_CXXFLAGS += /std:c++latest
}

gcc:{
    QMAKE_CXXFLAGS += \
    -std=c++2a \
    -Wno-deprecated-copy
}

win32:RC_FILE = main_icon/myapp.rc

SOURCES += \
    callout.cpp \
    chartview.cpp \
    datamodel.cpp \
    degreesmodel.cpp \
    main.cpp \
    mainwindow.cpp \
    polynomial.cpp \
    tableview.cpp

HEADERS += \
    callout.h \
    chartview.h \
    ctre.hpp \
    datamodel.h \
    degreesmodel.h \
    mainwindow.h \
    polynomial.h \
    tableview.h \
    types.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/res.qrc
