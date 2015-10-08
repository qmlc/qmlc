#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include "extmodel.h"
#include "submodel.h"
//#include "ucmodel.h"


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    qmlRegisterExtendedType<QQuickItem,ExtModel>("AppType", 1, 0, "ExtModel");
    qmlRegisterExtendedType<ExtModel,SubModel>("AppType", 1, 0, "SubModel");
    //qmlRegisterExtendedUncreatableType<QQuickItem,UcModel>("AppType", 1, 0, "UcModel", "Not available.");

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/apptype.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
