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

SOURCES += \
    callout.cpp \
    coeffmodel.cpp \
    datamodel.cpp \
    main.cpp \
    mainwindow.cpp \
    polynomial.cpp \
    view.cpp

HEADERS += \
    callout.h \
    coeffmodel.h \
    ctre.hpp \
    datamodel.h \
    mainwindow.h \
    polynomial.h \
    types.h \
    view.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
