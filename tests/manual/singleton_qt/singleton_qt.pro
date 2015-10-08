TEMPLATE = subdirs
SUBDIRS += app
# Note that the type info library needs the DEFINES to be set here.
# Only the DEFINES that affect the headers listed in TYPEINFO are needed.
qmc {
    # This makes the support library for types registered in app so that QML
    # code can be compiled.
    TYPELIB = __qmctypelib
    TYPEINFO = app/onemodel.h OneModel singleton AppType 1 0 OneModel QObject
    TYPEINFO += app/twomodel.h TwoModel singleton,cb_TwoModel AppType 1 0 TwoModel direct:app/twomodel.cpp
    !system(qmc-typelibpro.py $$TYPELIB \"core qml quick\" \"$$DEFINES \" $$TYPEINFO) {
        error("Failed to generate project file for $$TYPELIB")
    }
    SUBDIRS += $$TYPELIB
    app.depends = $$TYPELIB
}
