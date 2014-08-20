TARGET = plugins
QT += qml quick

QMLCBASEPATH = ../../../

INCLUDEPATH += $$QMLCBASEPATH/qmcloader
LIBS += -L$$QMLCBASEPATH/qmcloader
LIBS += -lqmcloader

# Avoid going to debug/release subdirectory
# so that our application will see the
# import path for the Charts module.
win32: DESTDIR = ./

DESTPATH=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path=$$DESTPATH
compiled_files.files = app.qmc
compiled_files.path=$$DESTPATH

INSTALLS += target compiled_files

SOURCES += main.cpp
RESOURCES += app.qrc
