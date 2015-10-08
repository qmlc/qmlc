TEMPLATE = app
QT += qml quick
TARGET = test
RESOURCES += res.qrc
QML_FILES = main.qml SomeType.qml

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
    qmc_res.target = _qrc_res.cpp
    qmc_res.path = $$OUT_PWD
    qmc_res.commands = qmc-rcc.sh \"$$QMCFLAGS \" $$OUT_PWD/$$qmc_res.target $$RCF $$QMAKE_RESOURCE_FLAGS
    QMAKE_EXTRA_TARGETS += qmc_res
    QMAKE_CLEAN += $$qmc_res.target
    SOURCES += $$qmc_res.target
}
