QT += qml quick

QMLCBASEPATH = ../../../
PROJECTBASEPATH = ../

INCLUDEPATH += $$QMLCBASEPATH/qmccompiler
LIBS += -L$$QMLCBASEPATH/qmccompiler
LIBS += -lqmccompiler

# These are just for using JIT
QT += qml-private core-private
INCLUDEPATH += $$QMLCBASEPATH/3rdparty/masm
INCLUDEPATH += $$QMLCBASEPATH/3rdparty/masm/stubs
INCLUDEPATH += $$QMLCBASEPATH/3rdparty/masm/stubs/wtf
INCLUDEPATH += $$QMLCBASEPATH/3rdparty/masm/jit
INCLUDEPATH += $$QMLCBASEPATH/3rdparty/masm/disassembler
include($$QMLCBASEPATH/3rdparty/masm/masm-defs.pri)
DEFINES += ENABLE_JIT ASSERT_DISABLED=1

SOURCES += main.cpp \
           $$PROJECTBASEPATH/cppsubitem.cpp

HEADERS += $$PROJECTBASEPATH/cppsubitem.h

RESOURCES += res.qrc

TARGET = compile

# run the target that was compiled to compile the .qml etc. files
QMAKE_POST_LINK = LD_LIBRARY_PATH=$$QMLCBASEPATH/qmccompiler ./compile

# clean the compiled files(TODO: improve, from .qrc?)
QMAKE_CLEAN = ../*.qmc
