TEMPLATE = subdirs
CONFIG += ordered
CONFIG += qmc


TYPELIB = __qmctypelib
TYPEINFO = testlistmodel.h TestListModel noreg unused 1 0 unused QAbstractListModel
TYPEINFO += testlistmodel2.h TestListModel2 reg TestListModel2 1 0 TestListModel2 TestListModel_1_0.h
!system(qmc-typelibpro.py $$TYPELIB \"core gui qml quick dbus feedback\"  \"$$DEFINES \" $$TYPEINFO) {
    error("Failed to generate project file for $$TYPELIB")
}

SUBDIRS += $$TYPELIB
SUBDIRS += src_compiled.pro
