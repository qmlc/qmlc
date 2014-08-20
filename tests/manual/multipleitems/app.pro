QT += qml quick

QMLCBASEPATH = ../../../

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

DESTPATH=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path = $$DESTPATH

compiled_files.files =  qml/multipleitems.qmc
compiled_files.path = $$DESTPATH/qml

compiled_files2.files =  qml/content/QmlSubItem.qmc \
                        qml/content/CompositeItem.qmc \
                        qml/content/testscript1.jsc
compiled_files2.path = $$DESTPATH/qml/content

INSTALLS += target compiled_files compiled_files2
