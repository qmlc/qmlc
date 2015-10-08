#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include "onemodel.h"


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    qmlRegisterSingletonType("AppType", 1, 0, "OneModel", &qjsvalue_provider);

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/apptype.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
