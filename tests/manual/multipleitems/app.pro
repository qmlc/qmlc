
QT += qml quick

SOURCES += main.cpp

HEADERS +=

RESOURCES += res.qrc

TARGET = multipleitems

DEST_DIR=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path = $$DEST_DIR

INSTALLS += target

### qmc start

CONFIG += qmc

CONFIG(qmc){

    # these are only needed because we are building in the same build as qmlc
    QMLC_BASE_DIR = ../../../
    QMAKE_POST_LINK += export PATH=$$PWD/$$QMLC_BASE_DIR/qmc:$$PATH;
    QMAKE_POST_LINK += export LD_LIBRARY_PATH=$$PWD/$$QMLC_BASE_DIR/qmccompiler:$$PWD/$$QMLC_BASE_DIR/qmcloader;

    QMLC_DEST_DIR = $$DEST_DIR
    QMLC_QML = app.qml \
                 qml/QmlJSItems.qml \
                 qml/content/QmlSubItem.qml \
                 qml/content/testscript1.js

    #QMLC_QML_BASE_DIR =
    #QMLC_EXIT_ON_ERROR = false
    #QMLC_QML2_IMPORT_PATH =

    include($$QMLC_BASE_DIR/qmlc.pri)
}

### qmc end
