TEMPLATE = subdirs
CONFIG += ordered
CONFIG += qmc

TYPELIB = __qmctypelib
TYPEINFO = testlistmodel.h TestListModel reg TestListModel 1 0 TestListModel QAbstractListModel
!system(qmc-typelibpro.py $$TYPELIB \"core gui qml quick dbus feedback\"  \"$$DEFINES \" $$TYPEINFO) {
    error("Failed to generate project file for $$TYPELIB")
}

SUBDIRS += $$TYPELIB
SUBDIRS += src_compiled.pro


