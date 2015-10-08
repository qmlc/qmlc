TEMPLATE = app
TARGET = apptest_loader
QT += qml quick

CONFIG += link_pkgconfig

target.path = $$TARGET_DIR/bin
INSTALLS += target

SOURCES += main_loader.cpp \
    testlistmodel.cpp testlistmodel2.cpp 


HEADERS += testlistmodel.h testlistmodel2.h 

packagesExist(qdeclarative5-boostable) {
    message("Building with qdeclarative-boostable support")
    DEFINES += HAS_BOOSTER
    PKGCONFIG += qdeclarative5-boostable
} else {
    warning("qdeclarative-boostable not available; startup times will be slower")
}

QMLC_DEST_DIR = $$TARGET_DIR/bin
QMLC_QML2_IMPORT_PATH += $$TARGET_DIR_PLUGIN
QML2_IMPORT_PATH += $$TARGET_DIR_PLUGIN
QMCFLAGS = -t $$OUT_PWD/__qmctypelib/lib__qmctypelib

QMLC_QML = \
    ../qml/main.qml

CONFIG += qmc
LIBS += -lqmcloader

include(/usr/share/qt5/mkspecs/features/qmlc.pri)



