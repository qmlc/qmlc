TARGET = plugins
QT += qml quick

QMLCBASEPATH = ../../../

INCLUDEPATH += $$QMLCBASEPATH/qmcloader
LIBS += -L$$QMLCBASEPATH/qmcloader
LIBS += -lqmcloader

# Avoid going to debug/release subdirectory
# so that our application will see the
# import path for the Charts module.
DESTDIR = ./

DESTPATH=$$[QT_INSTALL_TESTS]/qmlc/manual/$$TARGET

target.path=$$DESTPATH

qmc_files.path = $$DESTPATH
qmc_files.commands = $$QMAKE_COPY app.qmc $(INSTALL_ROOT)$$DESTPATH

INSTALLS += target qmc_files

SOURCES += main.cpp
RESOURCES += app.qrc

QMAKE_POST_LINK += LD_LIBRARY_PATH=$$QMLCBASEPATH/qmccompiler PATH=$$QMLCBASEPATH/qmc qmc app.qml

#QMAKE_CLEAN = $$PROJECTBASEPATH/*.qmc $$PROJECTBASEPATH/*.jsc
