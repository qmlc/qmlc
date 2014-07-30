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

#include <QTest>
#include <QQmlComponent>
#include <QQmlEngine>

#include "testcreatefile.h"
#include "qmlc.h"
#include "scriptc.h"
#include "qmcloader.h"

#define SUB_ITEM_QMC "SubItem.qmc"
#define SUB_ITEM_WITH_SCRIPT_QMC "SubItemWithScript.qmc"
#define TEST_SCRIPT_1_JSC "testscript1.jsc"
#define TEST_SCRIPT_2_JSC "testscript2.jsc"

#define TEST_MOD_1_QMC "testmod1.qmc"

#define TEST_MOD_2_QMC "testmod2.qmc"

#include <private/qqmlcompiler_p.h>
#include <private/qqmlcomponent_p.h>

TestCreateFile::TestCreateFile(QObject *parent) :
    QObject(parent),
    tempDir(NULL)
{
}

TestCreateFile::~TestCreateFile()
{
}

/*
 * This is simple test case to test loading a file with filename
 */
void TestCreateFile::testLoadSingleFile()
{
    QQmlEngine *engine = new QQmlEngine;
    QmcLoader loader(engine);
    QQmlComponent *c = loader.loadComponent(tempDirPath(SUB_ITEM_QMC));
    QVERIFY(c);
    QObject *obj = c->create();
    QVariant var = obj->property("height");
    QVERIFY(!var.isNull());
    QVERIFY(var.toInt() == 20);
    delete obj;
    delete c;
    delete engine;
}

/*
 * This is test case that loads dependency automatically. Both success and fail case.
 */
void TestCreateFile::testLoadDependency()
{
    QQmlEngine *engine = new QQmlEngine;
    QmcLoader loader(engine);
    loader.setLoadDependenciesAutomatically(false);
    QQmlComponent *c = NULL;
    c = loader.loadComponent(tempDirPath(SUB_ITEM_WITH_SCRIPT_QMC));
    QVERIFY(!c);
    loader.setLoadDependenciesAutomatically(true);
    c = loader.loadComponent(tempDirPath(SUB_ITEM_WITH_SCRIPT_QMC));
    QVERIFY(c);
    QObject *obj = c->create();
    QVariant var = obj->property("height");
    QVERIFY(!var.isNull());
    QVERIFY(var.toInt() == 40);
    delete obj;
    delete c;
    delete engine;
}

void TestCreateFile::testLoadModule1()
{
    QQmlEngine *engine = new QQmlEngine;
    QmcLoader loader(engine);
    QQmlComponent *c = loader.loadComponent(tempDirPath(TEST_MOD_1_QMC));
    QVERIFY(c);
    QObject *myObject = c->create();
    QVariant val;
    val = myObject->property("width");
    QVERIFY(val.isValid() && !val.isNull());
    QVERIFY(val.toInt() == 100);

    delete myObject;
    delete c;
    delete engine;
}

void TestCreateFile::testLoadModule2()
{
    QQmlEngine *engine = new QQmlEngine;
    QmcLoader loader(engine);
    QQmlComponent *c = loader.loadComponent(tempDirPath(TEST_MOD_2_QMC));
    QVERIFY(c);
    QQmlComponentPrivate *cPriv = QQmlComponentPrivate::get(c);
    QVERIFY(cPriv);
    QVERIFY(cPriv->cc);
    QVERIFY(cPriv->cc->scripts.size() == 2);
    QObject *myObject = c->create();
    QVariant val;
    val = myObject->property("m1h");
    QVERIFY(val.isValid() && !val.isNull());
    QVERIFY(val.toInt() == 200);
    val = myObject->property("m2w");
    QVERIFY(val.isValid() && !val.isNull());
    QVERIFY(val.toInt() == 30);

    delete myObject;
    delete c;
    delete engine;
}

void TestCreateFile::initTestCase()
{
    tempDir = new QTemporaryDir;
    QVERIFY(tempDir);
    QVERIFY(tempDir->isValid());
    QVERIFY(tempDir->autoRemove());
    QVERIFY(!tempDir->path().isEmpty());

    QDir dir (tempDir->path());

    qDebug() << "Created temp dir" << tempDir->path();

    QQmlEngine *engine = new QQmlEngine;
    bool ret;
    QmlC qmlc(engine);
    qmlc.setBasePath("");

    ScriptC scriptc(engine);
    scriptc.setBasePath("");

    ret = qmlc.compile("qrc:/testqml/SubItem.qml", tempDirPath(SUB_ITEM_QMC));
    QVERIFY(ret);
    ret = qmlc.compile("qrc:/testqml/SubItemWithScript.qml", tempDirPath(SUB_ITEM_WITH_SCRIPT_QMC));
    QVERIFY(ret);

    ret = scriptc.compile("qrc:/testqml/testscript1.js", tempDirPath(TEST_SCRIPT_1_JSC));
    QVERIFY(ret);
    ret = scriptc.compile("qrc:/testqml/testscript2.js", tempDirPath(TEST_SCRIPT_2_JSC));
    QVERIFY(ret);

    // testmod 1

    ret = qmlc.compile("qrc:/testqml/testmod1.qml", tempDirPath(TEST_MOD_1_QMC));
    QVERIFY(ret);

    ret = dir.mkdir("modsimple");
    QVERIFY(ret);

    ret = qmlc.compile("qrc:/testqml/modsimple/SimpleItem.qml", tempDirPath("modsimple/SimpleItem.qmc"));
    QVERIFY(ret);

    // testmod 2

    ret = qmlc.compile("qrc:/testqml/testmod2.qml", tempDirPath(TEST_MOD_2_QMC));
    QVERIFY(ret);

    ret = dir.mkdir("mod");
    QVERIFY(ret);

    ret = qmlc.compile("qrc:/testqml/mod/ModItem.qml", tempDirPath("mod/ModItem.qmc"));
    QVERIFY(ret);

    ret = qmlc.compile("qrc:/testqml/mod/ModItem11.qml", tempDirPath("mod/ModItem11.qmc"));
    QVERIFY(ret);

    ret = scriptc.compile("qrc:/testqml/mod/mod.js", tempDirPath("mod/mod.jsc"));
    QVERIFY(ret);

    ret = QFile::copy(":/testqml/mod/qmldir", tempDirPath("mod/qmldir"));
    QVERIFY(ret);

    delete engine;
}

void TestCreateFile::cleanupTestCase()
{
    QDir t(tempDir->path());
    QVERIFY(t.exists());
    delete tempDir;
    tempDir = NULL;
    QVERIFY(!t.exists());
}

QString TestCreateFile::tempDirPath(const QString &file)
{
    return tempDir->path() + "/" + file;
}
