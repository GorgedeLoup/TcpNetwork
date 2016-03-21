#-------------------------------------------------
#
# Project created by QtCreator 2016-02-29T16:13:18
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = CTcpNetwork
TEMPLATE = lib

CONFIG += c++11

INCLUDEPATH += ../lib

DEFINES += CTCPNETWORK_LIBRARY

SOURCES += ctcpnetwork.cpp

HEADERS += ctcpnetwork.h\
        ctcpnetwork_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
