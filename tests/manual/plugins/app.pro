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

SOURCES += main.cpp
RESOURCES += app.qrc
