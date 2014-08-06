QT += qml quick

INCLUDEPATH += ../../qmcloader
LIBS += -L../../qmcloader
LIBS += -lqmcloader

# These are just for using JIT
QT += qml-private core-private
INCLUDEPATH += ../../3rdparty/masm
INCLUDEPATH += ../../3rdparty/masm/stubs
INCLUDEPATH += ../../3rdparty/masm/stubs/wtf
INCLUDEPATH += ../../3rdparty/masm/jit
INCLUDEPATH += ../../3rdparty/masm/disassembler
include(../../3rdparty/masm/masm-defs.pri)
DEFINES += ENABLE_JIT ASSERT_DISABLED=1

SOURCES += main.cpp \
           dataobject.cpp
HEADERS += dataobject.h \
    cputimer.h

OTHER_FILES += \
    view.qmc \
    view.qml

RESOURCES += objectlistmodel.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/quick/models/objectlistmodel
INSTALLS += target
