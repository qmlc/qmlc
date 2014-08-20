TARGET = compile
QT += qml quick

QMLCBASEPATH = ../../../../
PROJECTBASEPATH = ../import

INCLUDEPATH += $$QMLCBASEPATH/qmccompiler $$PROJECTBASEPATH
LIBS += -L$$QMLCBASEPATH/qmccompiler
LIBS += -lqmccompiler

# Avoid going to debug/release subdirectory
# so that our application will see the
# import path for the Charts module.
win32: DESTDIR = ./

HEADERS += $$PROJECTBASEPATH/piechart.h \
           $$PROJECTBASEPATH/pieslice.h \
           $$PROJECTBASEPATH/chartsplugin.h

SOURCES += main.cpp \
           $$PROJECTBASEPATH/piechart.cpp \
           $$PROJECTBASEPATH/pieslice.cpp \
           $$PROJECTBASEPATH/chartsplugin.cpp

RESOURCES += $$PROJECTBASEPATH/res.qrc

# run the target that was compiled to compile the .qml etc. files
QMAKE_POST_LINK = LD_LIBRARY_PATH=$$(LD_LIBRARY_PATH):$$QMLCBASEPATH/qmccompiler ./compile

# clean the compiled files(TODO: improve, from .qrc?)
#QMAKE_CLEAN = $$PROJECTBASEPATH/*.qmc $$PROJECTBASEPATH/*.jsc

