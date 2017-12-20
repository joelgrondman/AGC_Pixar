#-------------------------------------------------
#
# Project created by QtCreator 2017-12-08T16:31:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pixar
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    objfile.cpp \
    mesh.cpp \
    meshtools.cpp \
    mainview.cpp

HEADERS  += mainwindow.h \
    mesh.h \
    meshtools.h \
    objfile.h \
    vertex.h \
    halfedge.h \
    face.h \
    mainview.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
