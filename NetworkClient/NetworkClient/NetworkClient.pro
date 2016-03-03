#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:28:15
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetworkClient
TEMPLATE = app

INCLUDEPATH += ../lib\
               ../lib/CTcpNetwork\
               ../lib/config\

LIBS += ../bin/CTcpNetwork.dll

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
