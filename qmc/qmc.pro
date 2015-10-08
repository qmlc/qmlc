#-------------------------------------------------
#
# Project created by QtCreator 2014-07-14T21:10:27
#
#-------------------------------------------------
include(../config.pri)

QT       += core qml qml-private core-private
QT       += gui quick

TARGET = qmc
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../qmccompiler
INCLUDEPATH += ../3rdparty/masm
INCLUDEPATH += ../3rdparty/masm/stubs
INCLUDEPATH += ../3rdparty/masm/stubs/wtf
INCLUDEPATH += ../3rdparty/masm/jit
INCLUDEPATH += ../3rdparty/masm/disassembler
INCLUDEPATH += ../include

include(../3rdparty/masm/masm-defs.pri)
DEFINES += ENABLE_JIT ASSERT_DISABLED=1
DEFINES += DEBUG_QMC

LIBS += -L../qmccompiler
LIBS += -lqmccompiler

SOURCES += main.cpp \
    comp.cpp

HEADERS += \
    comp.h

unix {
    qmcrcc.files = qmc-rcc.sh
    qmcrcc.path = $$BINDIR

    qmcrccpro.files = qmc-rccpro.py
    qmcrccpro.path = $$BINDIR

    qmccpptype.files = qmc-typelibpro.py
    qmccpptype.path = $$BINDIR

    target.path = $$BINDIR
    INSTALLS += target qmcrcc qmccpptype qmcrccpro
}
