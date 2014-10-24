TEMPLATE = lib
CONFIG += plugin
QT += qml quick

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
qmldir.files=$$PWD/qmldir
qmldir.path=$$DESTPATH
qml_files.files=$$PWD/QmlInPlugin.qml
qml_files.path=$$DESTPATH

INSTALLS += target qmldir qml_files

RESOURCES += res.qrc

# Copy the qmldir file and qml files to the same folder as the plugin binary
# cause we need it for running without installing and also for compiling
# ../app.qml
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$_PRO_FILE_PWD_/qmldir) $$DESTDIR), /, $$QMAKE_DIR_SEP)
QMAKE_POST_LINK += ;
QMAKE_POST_LINK += $$QMAKE_COPY $$replace($$list($$quote($$_PRO_FILE_PWD_/*.qml) $$DESTDIR), /, $$QMAKE_DIR_SEP)
QMAKE_POST_LINK += ;


### qmc start

CONFIG += qmc

# these are only needed because we are building in the same build as qmlc
# they are not needed if qmlc is already installed in default paths
QMLC_BASE_DIR = ../../../../
QMAKE_POST_LINK += export PATH=$$PWD/$$QMLC_BASE_DIR/qmc:$(PATH);
QMAKE_POST_LINK += export LD_LIBRARY_PATH=$$PWD/$$QMLC_BASE_DIR/qmccompiler;

# we want to install into DESTDIR after building before installing so
# ../app.pro that 'imports Charts' can be build
QMLC_TMP_DEST_DIR = $$DESTDIR

# these will always be needed for a plugin
QMLC_DEST_DIR = $$DESTPATH
QMLC_QML = QmlInPlugin.qml
QMLC_EXTRA = qmldir_loader

include($$QMLC_BASE_DIR/qmlc.pri)
