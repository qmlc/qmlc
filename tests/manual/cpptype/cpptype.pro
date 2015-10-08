TEMPLATE = subdirs
CONFIG += ordered
# Note that the type info library needs the DEFINES to be set here.
# Only the DEFINES that affect the headers listed in TYPEINFO are needed.
qmc {
    # This makes the support library for types registered in app so that QML
    # code can be compiled.
    TYPELIB = __qmctypelib
    TYPEINFO = modeltest.h ModelTest reg AppTypeTest 1 0 ModelTest QObject,submodel.h,SubModel_1_0.h
    TYPEINFO += submodel.h SubModel reg AppTypeTest 1 0 SubModel QObject
    TYPEINFO += ucmodel.h UcModel reg,uc AppTypeTest 1 0 UcModel QObject
    TYPEINFO += unused unused na AppTypeTest 1 0 NaModel unused
    TYPEINFO += simplemodel.h SimpleModel reg AppTypeTest 1 0 SimpleModel direct:simplemodel.cpp
    !system(qmc-typelibpro.py $$TYPELIB \"core qml\" \"$$DEFINES \" $$TYPEINFO) {
        error("Failed to generate helper library $$TYPELIB")
    }
    SUBDIRS += $$TYPELIB
}
SUBDIRS += cpptype_app.pro
