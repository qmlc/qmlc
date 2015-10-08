include(app.pri)

SOURCES += main_loader.cpp

TARGET = resources_loader
target.path = $$DEST_DIR
INSTALLS += target

# Compile qml/js

QMCFLAGS =
DEFINES += QMC
LIBS += -lqmcloader
declarative_debug|qml_debug {
    QMCFLAGS += -g
    DEFINES += QMC_DEBUG
    LIBS += -lqmcdebugger
}
!system(qmc-rccpro.py __qmc-res.pri $$PWD $$OUT_PWD RESOURCES \" \" \"$$QMCFLAGS \" $$RESOURCES) {
    error("Resource file handling for compiled QML failed, RESOURCES.")
}
include(__qmc-res.pri)

# Handles external resource file one at a time. Needs compilation
!system(qmc-rccpro.py __qmc-res3.pri $$PWD $$OUT_PWD EXT_RES3 RES3DEPS \"$$QMCFLAGS \" $$EXT_RES3) {
    error("Resource file handling for compiled QML failed, EXT_RES3.")
}
include(__qmc-res3.pri)
# Avoid over-writing the non-compiled QML resource file.
res3.target = qmc_$$res3.target
res3.depends += $$EXT_RES3 $$RES3DEPS
res3.commands = rcc -binary $$EXT_RES3 -o $$res3.target

# Handles external resource file one at a time. Needs no compilation
!system(qmc-rccpro.py __qmc-res5.pri $$PWD $$OUT_PWD EXT_RES5 RES5DEPS \"$$QMCFLAGS \" $$EXT_RES5) {
    error("Resource file handling for compiled QML failed, EXT_RES5.")
}
include(__qmc-res5.pri)
# Avoid over-writing the non-compiled QML resource file.
res5.target = qmc_$$res5.target
res5.depends += $$EXT_RES5 $$RES5DEPS
res5.commands = rcc -binary $$EXT_RES5 -o $$res5.target

PRE_TARGETDEPS += $$res3.target $$res5.target
QMAKE_DISTCLEAN += $$res3.target $$res5.target

rcc_inst.files = $$res3.target $$res5.target
rcc_inst.path = $$DEST_DIR
INSTALLS += rcc_inst
