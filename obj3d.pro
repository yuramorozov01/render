QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

CONFIG += debug
#QMAKE_CXXFLAGS_DEBUG *= -pg
#QMAKE_LFLAGS_DEBUG *= -pg

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    barrier.cpp \
    barrierdata.cpp \
    main.cpp \
    mainwindow.cpp \
    model.cpp \
    objreader.cpp \
    workerthread.cpp

HEADERS += \
    barrier.h \
    barrierdata.h \
    mainwindow.h \
    model.h \
    objreader.h \
    workerthread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
