TEMPLATE = subdirs
SUBDIRS += app
# Note that the type info library needs the DEFINES to be set here.
# Only the DEFINES that affect the headers listed in TYPEINFO are needed.
qmc {
    # This makes the support library for types registered in app so that QML
    # code can be compiled.
    TYPELIB = __qmctypelib
    TYPEINFO = app/extmodel.h ExtModel QQuickItem AppType 1 0 ExtModel QObject,QQuickItem
    TYPEINFO += app/submodel.h SubModel ExtModel_1_0 AppType 1 0 SubModel QObject
    # Requires QT 5.4 or later.
    #TYPEINFO += app/ucmodel.h UcModel QQuickItem,uc AppType 1 0 SubModel QObject
    !system(qmc-typelibpro.py $$TYPELIB \"core qml quick\" \"$$DEFINES \" $$TYPEINFO) {
        error("Failed to generate helper library $$TYPELIB")
    }
    SUBDIRS += $$TYPELIB
    app.depends = $$TYPELIB
}
