#include <QtQml/QtQml>
#include <QtQuick/QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/main.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
