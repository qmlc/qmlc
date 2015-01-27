include(../config.pri)

QT       += qml qml-private core-private
QT       -= gui

VERSION = 0.0.1
CONFIG += create_pc create_prl no_install_prl

TARGET = qmcdebugger
TEMPLATE = lib

DEFINES += QMCDEBUGGER_LIBRARY

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../3rdparty/masm
INCLUDEPATH += ../3rdparty/masm/stubs
INCLUDEPATH += ../3rdparty/masm/stubs/wtf
INCLUDEPATH += ../3rdparty/masm/jit
INCLUDEPATH += ../3rdparty/masm/disassembler
INCLUDEPATH += ../include
INCLUDEPATH += ../qmcloader

include(../3rdparty/masm/masm-defs.pri)
DEFINES += ENABLE_JIT ASSERT_DISABLED=1

LIBS += -L../qmcloader
LIBS += -lqmcloader

SOURCES += qmcdebugger.cpp

HEADERS += qmcdebugger.h qmcdebugging.h


devheaders.files = qmcdebugging.h
devheaders.path = $$INCLUDEDIR/qmcdebugger

unix {
    target.path = $$LIBDIR
    INSTALLS += target devheaders
}

QMAKE_PKGCONFIG_PREFIX = $$PREFIX
QMAKE_PKGCONFIG_INCDIR = $$devheaders.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
