#-------------------------------------------------
#
# Project created by QtCreator 2011-03-22T02:23:06
#
#-------------------------------------------------

QT       += core gui network xml sql

TARGET = Serveur
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    socket.cpp \
    server.cpp \
    clientmanager.cpp \
    messages.cpp \
    depot.cpp \
    databasemanager.cpp \
    svnmanager.cpp

HEADERS  += widget.h \
    socket.h \
    server.h \
    clientmanager.h \
    messages.h \
    depot.h \
    databasemanager.h \
    svnmanager.h
