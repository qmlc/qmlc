#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T20:23:04
#
#-------------------------------------------------

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
devheaders.path = /usr/include/qmcloader

system("ln -s ../3rdparty")
INSTALL_HEADERS=$$system("find 3rdparty/ -name '*.h' -o -name '*pri'")

for(header, INSTALL_HEADERS) {
  hpath = /usr/include/qmcloader/$${dirname(header)}
  eval(headers_$${hpath}.files += $$header)
  eval(headers_$${hpath}.path = $$hpath)
  eval(INSTALLS *= headers_$${hpath})
}


unix {
    target.path = /usr/lib
    INSTALLS += target devheaders
}
QMAKE_PKGCONFIG_PREFIX = /usr
QMAKE_PKGCONFIG_INCDIR = $$devheaders.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
