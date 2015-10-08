#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include "modeltest.h"
#include "submodel.h"
#include "ucmodel.h"
#include "simplemodel.h"


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    qmlRegisterType<SubModel>("AppTypeTest", 1, 0, "SubModel");
    qmlRegisterType<ModelTest>("AppTypeTest", 1, 0, "ModelTest");
    qmlRegisterUncreatableType<UcModel>("AppTypeTest", 1, 0, "UcModel", "N/A");
    qmlRegisterTypeNotAvailable("AppTypeTest", 1, 0, "NaModel", "N/A");
    qmlRegisterType<SimpleModel>("AppTypeTest", 1, 0, "SimpleModel");

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/apptype.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
