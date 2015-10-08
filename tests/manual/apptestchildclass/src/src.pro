TEMPLATE = subdirs
CONFIG += ordered
CONFIG += qmc


TYPELIB = __qmctypelib
TYPEINFO = testlistmodel.h TestListModel noreg unused 1 0 unused QAbstractListModel
TYPEINFO += testlistmodel.h TestListModel2 reg TestListModel2 1 0 TestListModel2 TestListModel_1_0.h
TYPEINFO += testlistmodel.h TestAudioOutput reg TestAudioOutput 1 0 TestAudioOutput QAudioOutput
!system(qmc-typelibpro.py $$TYPELIB \"core gui qml quick dbus feedback multimedia\"  \"$$DEFINES \" $$TYPEINFO) {
    error("Failed to generate project file for $$TYPELIB")
}

SUBDIRS += $$TYPELIB
SUBDIRS += src_compiled.pro
