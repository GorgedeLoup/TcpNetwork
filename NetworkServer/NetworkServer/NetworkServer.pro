#-------------------------------------------------
#
# Project created by QtCreator 2016-03-02T18:18:53
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetworkServer
TEMPLATE = app

CONFIG += c++11

INCLUDEPATH += ../lib\
               ../lib/CTcpNetwork\
               ../lib/config\

LIBS += ../bin/CTcpNetwork.dll

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
