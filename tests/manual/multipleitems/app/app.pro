QT += qml quick

QMLCBASEPATH = ../../../../

INCLUDEPATH += $$QMLCBASEPATH/qmcloader
LIBS += -L$$QMLCBASEPATH/qmcloader
LIBS += -lqmcloader

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
           cppsubitem.cpp

HEADERS += cppsubitem.h

OTHER_FILES += \
    multipleitems.qmc \
    QmlSubItem.qmc \
    testscript1.jsc

RESOURCES += res.qrc

TARGET = multipleitems
target.path = $$[QT_INSTALL_TESTS]/quick/multipleitems

compiled_files.files =  qml/multipleitems.qmc qml/QmlSubItem.qmc qml/testscript1.jsc
compiled_files.path = $$[QT_INSTALL_TESTS]/quick/multipleitems/qml

INSTALLS += target compiled_files
