#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QDebug>
#include "onemodel.h"
#include "twomodel.h"


static QObject *cb_OneModel(QQmlEngine *q, QJSEngine *j)
{
    Q_UNUSED(q)
    Q_UNUSED(j)
    return new OneModel();
}

Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    qmlRegisterSingletonType<OneModel>("AppType", 1, 0, "OneModel", &cb_OneModel);
    qmlRegisterSingletonType<TwoModel>("AppType", 1, 0, "TwoModel", &cb_TwoModel);

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/apptype.qml"));
    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
