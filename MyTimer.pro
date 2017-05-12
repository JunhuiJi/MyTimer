#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T19:09:48
#
#-------------------------------------------------

QT       += core gui \
            sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Timer
TEMPLATE = app

RESOURCES += \
    resource/Icon.qrc

FORMS += \
    form/MainGui.ui

DISTFILES += \
    resource/AppIcon.rc

HEADERS += \
    include/CreateDatabase.h \
    include/MainGui.h

SOURCES += \
    src/main.cpp \
    src/MainGui.cpp

INCLUDEPATH += \
    ./include \
    ./src \
