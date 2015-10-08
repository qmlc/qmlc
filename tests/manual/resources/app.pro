include(app.pri)

SOURCES += main.cpp

TARGET = resources
target.path = $$DEST_DIR
INSTALLS += target

res3.commands = rcc -binary $$EXT_RES3 -o $$res3.target
res5.commands = rcc -binary $$EXT_RES5 -o $$res5.target
PRE_TARGETDEPS += $$res3.target $$res5.target
QMAKE_DISTCLEAN += $$res3.target $$res5.target

rcc_inst.files = $$res3.target $$res5.target
rcc_inst.path = $$DEST_DIR
INSTALLS += rcc_inst
