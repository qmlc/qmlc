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
#define TEST_SCRIPT_1_QMC "testscript1.qmc"
#define TEST_SCRIPT_2_QMC "testscript2.qmc"

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

void TestCreateFile::initTestCase()
{
    tempDir = new QTemporaryDir;
    QVERIFY(tempDir);
    QVERIFY(tempDir->isValid());
    QVERIFY(tempDir->autoRemove());
    QVERIFY(!tempDir->path().isEmpty());

    qDebug() << "Created temp dir" << tempDir->path();

    bool ret;
    QmlC qmlc;
    qmlc.setBasePath("");
    ret = qmlc.compile("qrc:/testqml/SubItem.qml", tempDirPath(SUB_ITEM_QMC));
    QVERIFY(ret);
    ret = qmlc.compile("qrc:/testqml/SubItemWithScript.qml", tempDirPath(SUB_ITEM_WITH_SCRIPT_QMC));
    QVERIFY(ret);

    ScriptC scriptc;
    scriptc.setBasePath("");
    ret = scriptc.compile("qrc:/testqml/testscript1.js", tempDirPath(TEST_SCRIPT_1_QMC));
    QVERIFY(ret);
    ret = scriptc.compile("qrc:/testqml/testscript2.js", tempDirPath(TEST_SCRIPT_2_QMC));
    QVERIFY(ret);
}

void TestCreateFile::cleanupTestCase()
{
    delete tempDir;
    tempDir = NULL;
}

QString TestCreateFile::tempDirPath(const QString &file)
{
    return tempDir->path() + "/" + file;
}
