#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T20:23:04
#
#-------------------------------------------------

QT       += qml qml-private core-private
QT       -= gui

TARGET = qmcloader
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

DEFINES += QMCLOADER_LIBRARY

SOURCES += qmcloader.cpp \
    qmcunit.cpp \
    qmcunitpropertycachecreator.cpp \
    qmctypeunit.cpp \
    qmcscriptunit.cpp \
    qmctypeunitcomponentandaliasresolver.cpp \
    qmcbackedinstructionselection.cpp


HEADERS += qmcloader.h \
    qmcloader_global.h \
    compiler.h \
    qmcunit.h \
    qmcunitpropertycachecreator.h \
    qmctypeunit.h \
    qmcscriptunit.h \
    qmctypeunitcomponentandaliasresolver.h \
    qmcbackedinstructionselection.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
