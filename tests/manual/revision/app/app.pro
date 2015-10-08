TEMPLATE = app
QT += qml quick
TARGET = apptype
RESOURCES += res.qrc
SOURCES += revised.cpp
HEADERS += revised.h
QML_FILES = apptype.qml

!qmc {
    SOURCES += main.cpp
}

qmc {
    SOURCES += main_loader.cpp
    QMCFLAGS =
    DEFINES += QMC
    LIBS += -lqmcloader
    declarative_debug|qml_debug {
        QMCFLAGS += -g
        DEFINES += QMC_DEBUG
        LIBS += -lqmcdebugger
    }
    RCF =
    for (file, RESOURCES) {
        RCF += $$IN_PWD/$$file
    }
    RESOURCES =
    QMCFLAGS += -t $$OUT_PWD/../__qmctypelib/lib__qmctypelib
    qmc_res.target = _qrc_res.cpp
    qmc_res.path = $$OUT_PWD
    qmc_res.commands = qmc-rcc.sh \"$$QMCFLAGS \" $$OUT_PWD/$$qmc_res.target $$RCF $$QMAKE_RESOURCE_FLAGS
    QMAKE_EXTRA_TARGETS += qmc_res
    QMAKE_CLEAN += $$qmc_res.target
    SOURCES += $$qmc_res.target
}
