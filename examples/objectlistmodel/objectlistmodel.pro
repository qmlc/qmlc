QT += qml quick

INCLUDEPATH += ../../qmcloader
LIBS += -L../../qmcloader
LIBS += -lqmcloader

SOURCES += main.cpp \
           dataobject.cpp
HEADERS += dataobject.h

OTHER_FILES += \
    view.qmc \
    view.qml

RESOURCES += objectlistmodel.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/quick/models/objectlistmodel
INSTALLS += target
