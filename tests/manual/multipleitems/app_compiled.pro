
## compile loader

# the qmlc loader app we compile with everything the same qmake as the
# uncompiled app(app.pro) except we change SOURCES and TARGET to loader
# versions.

QT += qml quick

SOURCES += main_loader.cpp

HEADERS +=

RESOURCES += res.qrc

TARGET = multipleitems

DEST_DIR=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

TARGET = multipleitems_loader

target.path = $$DEST_DIR

INSTALLS += target

## compile qml/js

CONFIG += qmc

# these are only needed because we are building in the same build as qmlc
QMLC_BASE_DIR = ../../../
QMAKE_POST_LINK += export PATH=$$PWD/$$QMLC_BASE_DIR/qmc:$$PATH;
QMAKE_POST_LINK += export LD_LIBRARY_PATH=$$PWD/$$QMLC_BASE_DIR/qmccompiler:$$PWD/$$QMLC_BASE_DIR/qmcloader;

# these will always be needed to compile the qml/js of an application
QMLC_DEST_DIR = $$DEST_DIR
QMLC_QML = app.qml \
             qml/QmlJSItems.qml \
             qml/content/QmlSubItem.qml \
             qml/content/testscript1.js

include($$QMLC_BASE_DIR/qmlc.pri)
