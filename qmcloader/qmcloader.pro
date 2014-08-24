#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T20:23:04
#
#-------------------------------------------------

include(../config.pri)

QT       += qml qml-private core-private
QT       -= gui

VERSION = 0.0.1
CONFIG += create_pc create_prl no_install_prl

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
    qmcbackedinstructionselection.cpp \
    qmcloadingmeasurer.cpp \
    qrcloader.cpp

HEADERS += qmcloader.h \
    qmcloader_global.h \
    qmcunit.h \
    qmcunitpropertycachecreator.h \
    qmctypeunit.h \
    qmcscriptunit.h \
    qmctypeunitcomponentandaliasresolver.h \
    qmcbackedinstructionselection.h \
    qmcloadingmeasurer.h \
    qrcloader.h


devheaders.files = qmcloader.h qmcloader_global.h qrcloader.h
devheaders.path = $$INCLUDEDIR/qmcloader

unix {
    target.path = $$LIBDIR
    INSTALLS += target devheaders
}

QMAKE_PKGCONFIG_PREFIX = $$PREFIX
QMAKE_PKGCONFIG_INCDIR = $$devheaders.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
