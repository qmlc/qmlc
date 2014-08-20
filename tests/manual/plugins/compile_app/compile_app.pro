TARGET = compile
QT += qml quick

QMLCBASEPATH = ../../../../
PROJECTBASEPATH = ../

INCLUDEPATH += $$QMLCBASEPATH/qmccompiler $$PROJECTBASEPATH
LIBS += -L$$QMLCBASEPATH/qmccompiler
LIBS += -lqmccompiler

SOURCES += main.cpp
RESOURCES += $$PROJECTBASEPATH/app.qrc

# run the target that was compiled to compile the .qml etc. files
QMAKE_POST_LINK = LD_LIBRARY_PATH=$$(LD_LIBRARY_PATH):$$QMLCBASEPATH/qmccompiler ./compile

# clean the compiled files(TODO: improve, from .qrc?)
#QMAKE_CLEAN = $$PROJECTBASEPATH/*.qmc $$PROJECTBASEPATH/*.jsc

