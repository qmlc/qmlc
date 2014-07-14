/*!
 * Copyright (C) 2014 Nomovok Ltd. All rights reserved.
 * Contact: info@nomovok.com
 *
 * This file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * In addition, as a special exception, copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

#include <QtTest/QtTest>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>
#include <QQmlContext>
#include <QtQuick/QQuickItem>
#include <QString>
#include <QUrl>
#include "qmlc.h"
#include "jsc.h"
#include "qmcloader.h"
#include "compiler.h"
#include "testobject.h"

class SignalTester : public QObject
{
    Q_OBJECT
public:
    SignalTester(QObject *parent = NULL);
    virtual ~SignalTester();

    void sendSig();
    int val;

public slots:
    void rcvMsg(int v);

signals:
    void sig2();

};

SignalTester::SignalTester(QObject *parent)
    : QObject(parent),
      val(-1)
{
}

SignalTester::~SignalTester()
{
}

void SignalTester::sendSig()
{
    emit sig2();
}

void SignalTester::rcvMsg(int v)
{
    val = v;
}

class TestSimpleQmlLoad: public QObject
{
    Q_OBJECT
private slots:
    void loadListView1();
    void compileAndLoadListView1();

    void compileAndLoadSignal3();
    void loadSignal3();

    void compileAndLoadSignal2();
    void loadSignal2();

    void loadSignal1();
    void compileAndLoadSignal1();

    void compileAndLoadComponent1();
    void loadComponent1();

    void loadAlias1();
    void compileAndLoadAlias1();
    void loadSubItem2();
    void compileAndLoadSubItem2();
    void compileAndLoadSubItem1();

    void loadSubItem1();
    void compileAndLoadScript1();
    void compileAndLoadFunction1();
    void loadScript1();
    void loadFunction1();
    void compileAndLoadBinding1();
    void loadBinding1();
    void compileAndLoadItem();
    void loadItem();
    void initTestCase();
    void cleanupTestCase();

private:
    QQmlComponent *compileAndLoad(QQmlEngine *engine, const QString &file, const QList<QString> &dependencies = QList<QString>());
    QQmlComponent *load(QQmlEngine *engine, const QString &file);
    void printErrors(const QList<QQmlError>& errors);

};

void TestSimpleQmlLoad::initTestCase()
{
}

void TestSimpleQmlLoad::cleanupTestCase()
{
}

void TestSimpleQmlLoad::loadListView1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testlistview1.qml");

    QList<QObject*> l;
    l.append(new TestObject(1));

    engine->rootContext()->setContextProperty("model1", QVariant::fromValue(l));

    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadListView1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testlistview1.qml");

    QList<QObject*> l;
    l.append(new TestObject(1));

    engine->rootContext()->setContextProperty("model1", QVariant::fromValue(l));

    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadSignal3()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsignal3.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    SignalTester st;

    engine->rootContext()->setContextProperty("st", &st);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    st.sendSig();

    QVariant ret;
    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 10);

    st.sendSig();

    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 20);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadSignal3()
{
    const QString TEST_FILE(":/testqml/testsignal3.qml");

    QQmlEngine *engine = new QQmlEngine;
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    SignalTester st;

    engine->rootContext()->setContextProperty("st", &st);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    st.sendSig();

    QVariant ret;
    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 10);

    st.sendSig();

    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 20);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadSignal2()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsignal2.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);
    SignalTester st;

    engine->rootContext()->setContextProperty("st", &st);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    QVariant var = myObject->property("sigReceived");
    QVERIFY(!var.isNull());
    QVERIFY(var.toBool() == false);

    st.sendSig();
    var = myObject->property("sigReceived");
    QVERIFY(!var.isNull());
    QVERIFY(var.toBool() == true);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadSignal2()
{
    const QString TEST_FILE(":/testqml/testsignal2.qml");

    QQmlEngine *engine = new QQmlEngine;
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    SignalTester st;

    engine->rootContext()->setContextProperty("st", &st);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    QVariant var = myObject->property("sigReceived");
    QVERIFY(!var.isNull());
    QVERIFY(var.toBool() == false);

    st.sendSig();
    var = myObject->property("sigReceived");
    QVERIFY(!var.isNull());
    QVERIFY(var.toBool() == true);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadSignal1()
{
    const QString TEST_FILE(":/testqml/testsignal1.qml");

    QQmlEngine *engine = new QQmlEngine;
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    SignalTester st;

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    QVariant var = myObject->property("complete1");
    QVERIFY(!var.isNull());
    QVERIFY(var.toInt() == 12);

    QVERIFY(st.val == -1);
    QMetaObject::invokeMethod(myObject, "sendSig");
    QVERIFY(st.val == -1);

    QObject::connect(myObject, SIGNAL(sig1(int)), &st, SLOT(rcvMsg(int)));
    QMetaObject::invokeMethod(myObject, "sendSig");
    QVERIFY(st.val == 1);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadComponent1()
{
    const QString TEST_FILE(":/testqml/testcomponent1.qml");

    QQmlEngine *engine = new QQmlEngine;
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);
    QVariant var = myObject->property("c1");
    QVERIFY(!var.isNull());

    QVariant ret;
    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 10);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadSignal1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsignal1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);
    SignalTester st;

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);

    QVariant var = myObject->property("complete1");
    QVERIFY(!var.isNull());
    QVERIFY(var.toInt() == 12);

    QVERIFY(st.val == -1);
    QMetaObject::invokeMethod(myObject, "sendSig");
    QVERIFY(st.val == -1);

    QObject::connect(myObject, SIGNAL(sig1(int)), &st, SLOT(rcvMsg(int)));
    QMetaObject::invokeMethod(myObject, "sendSig");
    QVERIFY(st.val == 1);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadComponent1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testcomponent1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);
    QVariant var = myObject->property("c1");
    QVERIFY(!var.isNull());

    QVariant ret;
    QMetaObject::invokeMethod(myObject, "getSubWidth1", Q_RETURN_ARG(QVariant, ret));
    QVERIFY(ret.toInt() == 10);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadAlias1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testalias1.qml");
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);

    QObject* myObject = component->create();
    QVERIFY(myObject);
    QVariant v = myObject->property("w");
    QVERIFY(!v.isNull());
    QVERIFY(v.toInt() == 100);
    myObject->setProperty("width", QVariant(200));
    v = myObject->property("w");
    QVERIFY(v.toInt() == 200);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadAlias1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testalias1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QVERIFY(myObject != NULL);
    QVariant v = myObject->property("w");
    QVERIFY(!v.isNull());
    QVERIFY(v.toInt() == 100);
    myObject->setProperty("width", QVariant(200));
    v = myObject->property("w");
    QVERIFY(v.toInt() == 200);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::printErrors(const QList<QQmlError> &errors)
{
    foreach (QQmlError error, errors)
        qDebug() << "Error" << error;
}

QQmlComponent* TestSimpleQmlLoad::load(QQmlEngine *engine, const QString &file)
{
    QUrl url("qrc" + file);
    QQmlComponent* component = new QQmlComponent(engine, url, QQmlComponent::PreferSynchronous);

    if (!component)
        return NULL;

    if (component->isError() || !component->isReady()) {
        printErrors(component->errors());
        delete component;
        return NULL;
    }

    return component;
}

void TestSimpleQmlLoad::compileAndLoadSubItem2()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsubitem2.qml");
    QList<QString> dependencies;
    dependencies.append(":/testqml/testscript1.js");
    dependencies.append(":/testqml/testscript2.js");
    dependencies.append(":/testqml/SubItemWithScript.qml");
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE, dependencies);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int height = item->height();
    QVERIFY(height == 40);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadSubItem2()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsubitem2.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int height = item->height();
    QVERIFY(height == 40);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadSubItem1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsubitem1.qml");
    const QString TEST_DEPENDENCY(":/testqml/SubItem.qml");
    QList<QString> dependencies;
    dependencies.append(TEST_DEPENDENCY);
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE, dependencies);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 10);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadSubItem1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testsubitem1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 10);

    delete component;
    delete engine;
}


void TestSimpleQmlLoad::loadScript1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testscript1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 100);
    int height = item->height();
    QVERIFY(height == 200);
    item->setWidth(15);
    height = item->height();
    QVERIFY(height == 30);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadScript1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testscript1.qml");
    const QString TEST_SCRIPT(":/testqml/testscript1.js");
    QList<QString> dependencies;
    dependencies.append(TEST_SCRIPT);
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE, dependencies);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 100);
    int height = item->height();
    QVERIFY(height == 200);
    item->setWidth(15);
    height = item->height();
    QVERIFY(height == 30);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadItem()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testitem.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);

    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();  // width = 100
    QVERIFY(width == 100);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadItem()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testitem.qml");
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);
    QObject *myObject = component->create();
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();  // width = 100
    QVERIFY(width == 100);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadBinding1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testbinding1.qml");
    QQmlComponent* component = load(engine, TEST_FILE);
    QVERIFY(component);
    QObject* myObject = component->create();
    QVERIFY(myObject);
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int height = item->height();
    QVERIFY(height == 62);

    QList<QQuickItem *> childs = item->childItems();
    QVERIFY(childs.count() == 1);
    QQuickItem *child = childs[0];
    QVERIFY(child->height() == 62);

    item->setHeight(2);
    QVERIFY(child->height() == 2);

    delete component;
    delete engine;
}

void TestSimpleQmlLoad::loadFunction1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testfunction1.qml");
    QQmlComponent *component = load(engine, TEST_FILE);
    QVERIFY(component);
    QObject *myObject = component->create();
    QVERIFY(myObject);
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 100);
    int height = item->height();
    QVERIFY(height == width * 2);
    item->setWidth(220);
    height = item->height();
    QVERIFY(height == 20);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadFunction1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testfunction1.qml");
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);
    QObject *myObject = component->create();
    QVERIFY(myObject);
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int width = item->width();
    QVERIFY(width == 100);
    int height = item->height();
    //QVERIFY(height == width * 2);
    item->setWidth(220);
    height = item->height();
    QVERIFY(height == 20);
    delete component;
    delete engine;
}

void TestSimpleQmlLoad::compileAndLoadBinding1()
{
    QQmlEngine *engine = new QQmlEngine;
    const QString TEST_FILE(":/testqml/testbinding1.qml");
    QQmlComponent* component = compileAndLoad(engine, TEST_FILE);
    QVERIFY(component);
    QObject* myObject = component->create();
    QVERIFY(myObject);
    QQuickItem *item = qobject_cast<QQuickItem*>(myObject);
    int height = item->height();
    QVERIFY(height == 62);

    QList<QQuickItem *> childs = item->childItems();
    QVERIFY(childs.count() == 1);
    QQuickItem *child = childs[0];
    QVERIFY(child->height() == 62);

    item->setHeight(2);
    QVERIFY(child->height() == 2);
    delete component;
    delete engine;
}

QQmlComponent* TestSimpleQmlLoad::compileAndLoad(QQmlEngine *engine, const QString &file, const QList<QString> &dependencies)
{
    QString url("qrc" + file);
    QmlC c;
    QByteArray outputBuf;
    QDataStream output(&outputBuf, QIODevice::WriteOnly);
    bool success = c.compile(url, output);
    if (!success || outputBuf.length() < 500) {
        printErrors(c.errors());
        return NULL;
    }

    QList<QByteArray*> outputBufs;
    foreach (const QString & dependency, dependencies) {
        QByteArray *buf = new QByteArray;
        QDataStream out(buf, QIODevice::WriteOnly);
        bool ret = false;
        QString u("qrc" + dependency);
        if (dependency.endsWith(".js")) {
            JSC jsc;
            ret = jsc.compile(u, out);
            if (!ret)
                printErrors(jsc.errors());
        } else {
            QmlC qmlc;
            ret = qmlc.compile(u, out);
            if (!ret)
                printErrors(qmlc.errors());
        }
        if (!ret || buf->length() < 500) {
            return NULL;
        }
        outputBufs.append(buf);
    }

    QmcLoader loader(engine);

    // load first dependencies
    foreach (QByteArray *buf, outputBufs) {
        QDataStream in(buf, QIODevice::ReadOnly);
        success = loader.loadDependency(in);
        if (!success) {
            printErrors(loader.errors());
            return NULL;
        }
    }

    QDataStream input(&outputBuf, QIODevice::ReadOnly);
    QQmlComponent* component = loader.loadComponent(input);
    if (!component) {
        printErrors(loader.errors());
    }
    return component;
}

QTEST_MAIN(TestSimpleQmlLoad)
#include "testsimpleqmlload.moc"
