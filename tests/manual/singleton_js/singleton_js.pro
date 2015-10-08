TEMPLATE = subdirs
SUBDIRS += app
# Note that the type info library needs the DEFINES to be set here.
# Only the DEFINES that affect the headers listed in TYPEINFO are needed.
qmc {
    # This makes the support library for types registered in app so that QML
    # code can be compiled.
    TYPELIB = __qmctypelib
    TYPEINFO = unused unused jssingleton,qjsvalue_provider AppType 1 0 OneModel direct:app/onemodel.cpp
    !system(qmc-typelibpro.py $$TYPELIB \"core qml quick\" \"$$DEFINES \" $$TYPEINFO) {
        error("Failed to generate project file for $$TYPELIB")
    }
    SUBDIRS += $$TYPELIB
    app.depends = $$TYPELIB
}
