#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T20:44:03
#
#-------------------------------------------------

QT       += qml qml-private core-private
QT       -= gui

TARGET = qmccompiler
TEMPLATE = lib

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../3rdparty/masm
INCLUDEPATH += ../3rdparty/masm/stubs
INCLUDEPATH += ../3rdparty/masm/stubs/wtf
INCLUDEPATH += ../3rdparty/masm/jit
INCLUDEPATH += ../3rdparty/masm/disassembler
INCLUDEPATH += ../include

include(../3rdparty/masm/masm-defs.pri)

DEFINES += ENABLE_JIT ASSERT_DISABLED=1

DEFINES += QMCCOMPILER_LIBRARY

SOURCES += \
    qmlc.cpp \
    qmlcompilation.cpp \
    qmcexporter.cpp \
    jsc.cpp \
    compiler.cpp


HEADERS += qmccompiler_global.h \
    qmlc.h \
    qmlcompilation.h \
    qmcexporter.h \
    jsc.h \
    compiler.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}
