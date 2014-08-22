TARGET = compile
QT += qml quick

QMLCBASEPATH = ../../../../
PROJECTBASEPATH = ../import
VPATH = $$PROJECTBASEPATH

INCLUDEPATH += $$QMLCBASEPATH/qmccompiler $$PROJECTBASEPATH
LIBS += -L$$QMLCBASEPATH/qmccompiler
LIBS += -lqmccompiler

# Avoid going to debug/release subdirectory
# so that our application will see the
# import path for the Charts module.
win32: DESTDIR = ./

HEADERS += piechart.h \
           pieslice.h \
           chartsplugin.h

SOURCES += compile_plugin.cpp \
           piechart.cpp \
           pieslice.cpp \
           chartsplugin.cpp

RESOURCES += $$PROJECTBASEPATH/res.qrc

# run the target that was compiled to compile the .qml etc. files
QMAKE_POST_LINK = LD_LIBRARY_PATH=$$(LD_LIBRARY_PATH):$$QMLCBASEPATH/qmccompiler ./compile

# clean the compiled files(TODO: improve, from .qrc?)
#QMAKE_CLEAN = $$PROJECTBASEPATH/*.qmc $$PROJECTBASEPATH/*.jsc

