TEMPLATE = lib
CONFIG += plugin
QT += qml quick

QMLCBASEPATH = ../../../../

DESTDIR = ../Charts
TARGET = $$qtLibraryTarget(chartsplugin)

HEADERS += piechart.h \
           pieslice.h \
           chartsplugin.h

SOURCES += piechart.cpp \
           pieslice.cpp \
           chartsplugin.cpp

DESTPATH=$$[QT_INSTALL_TESTS]/qmlc/manual/multipleitems/Charts

target.path=$$DESTPATH
qmldir.files=$$PWD/qmldir $$PWD/qmldir_loader
qmldir.path=$$DESTPATH
qml_files.files=$$PWD/QmlInPlugin.qml
qml_files.path=$$DESTPATH

qmc_files.path = $$DESTPATH
qmc_files.commands = $$QMAKE_COPY QmlInPlugin.qmc $(INSTALL_ROOT)$$DESTPATH

INSTALLS += target qmldir qml_files qmc_files

RESOURCES += res.qrc


QMAKE_POST_LINK += LD_LIBRARY_PATH=$$QMLCBASEPATH/qmccompiler PATH=$$QMLCBASEPATH/qmc qmc QmlInPlugin.qml
QMAKE_POST_LINK += ;

# Copy the qmldir file and compiled files to the same folder as the plugin binary
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/qmldir) $$DESTDIR), /, $$QMAKE_DIR_SEP)
QMAKE_POST_LINK += ;
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/qmldir_loader) $$DESTDIR), /, $$QMAKE_DIR_SEP)
QMAKE_POST_LINK += ;
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/*.qmc) $$DESTDIR), /, $$QMAKE_DIR_SEP)
QMAKE_POST_LINK += ;
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$PWD/*.qml) $$DESTDIR), /, $$QMAKE_DIR_SEP)

