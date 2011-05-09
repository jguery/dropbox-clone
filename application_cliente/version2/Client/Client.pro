#-------------------------------------------------
#
# Project created by QtCreator 2011-03-20T15:36:08
#
#-------------------------------------------------

QT       += core gui network xml

TARGET = Client
TEMPLATE = app

CONFIG += crypto


SOURCES += main.cpp\
        widget.cpp \
    socket.cpp \
    messages.cpp \
    configurationdata.cpp \
    networkinterface.cpp \
    hddinterface.cpp \
    dir.cpp \
    file.cpp \
    media.cpp \
    depot.cpp \
    ciphertool.cpp

HEADERS  += widget.h \
    socket.h \
    messages.h \
    configurationdata.h \
    networkinterface.h \
    hddinterface.h \
    dir.h \
    file.h \
    media.h \
    depot.h \
    ciphertool.h
