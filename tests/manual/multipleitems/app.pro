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

SOURCES += main.cpp

HEADERS +=

RESOURCES += res.qrc

TARGET = multipleitems

DESTPATH=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path = $$DESTPATH

INSTALLS += target

QML_FILES = app.qml \
            qml/QmlJSItems.qml \
            qml/content/QmlSubItem.qml \
            qml/content/testscript1.js

CONFIG += qmc

qmc {
    QMAKE_POST_LINK += export PATH=$$PWD/$$QMLCBASEPATH/qmc:$$PATH;
    QMAKE_POST_LINK += export LD_LIBRARY_PATH=$$PWD/$$QMLCBASEPATH/qmccompiler;
    for(qmlfile, QML_FILES) {
       # compile
       contains(qmlfile, qmldir): next()
       contains(qmlfile, qmldir_loader): next()
       QMAKE_POST_LINK += cd ./$$dirname(qmlfile); qmc $$basename(qmlfile); cd -;

       # install
       qmcfile = $$replace(qmlfile, \\.qml, .qmc)
       qmcfile = $$replace(qmcfile, \\.js, .jsc)

       target = install_$$lower($$basename(qmlfile))
       target = $$replace(target, \\.qml, _qmc)
       target = $$replace(target, \\.js, _jsc)
       path = $${target}.path

       $$path = $$[QT_INSTALL_QML]/$$member(TARGETPATH, 0)
       commands = $${target}.commands
       $$commands += $$QMAKE_MKDIR $(INSTALL_ROOT)/$$DESTPATH/$$dirname(qmcfile);
       $$commands += $$QMAKE_COPY $$qmcfile $(INSTALL_ROOT)/$$DESTPATH/$$dirname(qmcfile);
       INSTALLS += $$target
    }
}

