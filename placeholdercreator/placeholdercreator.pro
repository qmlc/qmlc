#-------------------------------------------------
#
# Project created by QtCreator 2015-07-01T09:59:56
#
#-------------------------------------------------

include(../config.pri)

QT       += core
QT       -= gui
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
TARGET = qmc-cpptypeplaceholder
target.path = $$BINDIR
INSTALLS += target

SOURCES += cpptype.cpp \
    classparser.cpp \
    classparserfind.cpp \
    classparserinformation.cpp

HEADERS += \
    classparser.h \
    classparserfind.h \
    classparserinformation.h
