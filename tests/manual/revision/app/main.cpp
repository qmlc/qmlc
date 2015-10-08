#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>
#include "revised.h"


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    qmlRegisterType<Revised>("AppTypeTest", 1, 0, "Revised");
    qmlRegisterRevision<Revised,1>("AppTypeTest", 1, 1);
    qmlRegisterRevision<Revised,2>("AppTypeTest", 1, 2);

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/original.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
