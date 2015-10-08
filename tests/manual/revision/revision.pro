TEMPLATE = subdirs
SUBDIRS += app
# Note that the type info library needs the DEFINES to be set here.
# Only the DEFINES that affect the headers listed in TYPEINFO are needed.
qmc {
    # This makes the support library for types registered in app so that QML
    # code can be compiled.
    TYPELIB = __qmctypelib
    TYPEINFO = app/revised.h Revised reg,1,2 AppTypeTest 1 0 Revised QObject
    !system(qmc-typelibpro.py $$TYPELIB \"core qml\" \"$$DEFINES \" $$TYPEINFO) {
        error("Failed to make helper library in $$TYPELIB")
    }
    SUBDIRS += $$TYPELIB
    app.depends = $$TYPELIB
}
