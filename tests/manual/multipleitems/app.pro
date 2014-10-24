
QT += qml quick

SOURCES += main.cpp

HEADERS +=

RESOURCES += res.qrc

TARGET = multipleitems

DEST_DIR=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path = $$DEST_DIR

INSTALLS += target
