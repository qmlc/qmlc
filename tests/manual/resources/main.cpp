
#include <QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QResource>
#include <QDebug>


Q_DECL_EXPORT int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickView view;
    if (!QResource::registerResource("res3.rcc") ||
        !QResource::registerResource("res5.rcc"))
    {
        qDebug() << "Could not register resources.";
        return 1;
    }

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/app.qml"));

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));

    view.show();
    return app.exec();
}
