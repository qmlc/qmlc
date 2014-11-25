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

#ifndef TESTSIMPLEQMLLOAD_H
#define TESTSIMPLEQMLLOAD_H

#include <QString>
#include <QList>
#include <QQmlComponent>
#include <QQmlEngine>

class TestSimpleQmlLoad: public QObject
{
    Q_OBJECT
private slots:

    // first test needs to be compilation, loading will initialize structures
    void compileAndLoadBinding1();
    void loadBinding1();

    void loadItem();
    void compileAndLoadItem();

    void loadScript1();
    void compileAndLoadScript1();

    void loadSubItem2();
    void compileAndLoadSubItem2();

    void loadSubItem1();
    void compileAndLoadSubItem1();

    void loadSignal3();
    void compileAndLoadSignal3();

    void loadListView1();
    void compileAndLoadListView1();

    void loadSignal2();
    void compileAndLoadSignal2();

    void loadSignal1();
    void compileAndLoadSignal1();

    void loadComponent1();
    void compileAndLoadComponent1();

    void loadAlias1();
    void compileAndLoadAlias1();

    void loadAlias2();
    void compileAndLoadAlias2();

    void loadAlias3();
    void compileAndLoadAlias3();

    void loadAlias4();
    void compileAndLoadAlias4();

    void loadFunction1();
    void compileAndLoadFunction1();

    void loadModule1(); // compilation test in file based tests
    void compileModule1();

    void loadModule2(); // compilation test in file based tests
    void compileModule2();

    void loadCppSubItem();
    void compileAndLoadCppSubItem();

    void loadKeys();
    void compileAndLoadKeys();

    void loadProperty1();
    void compileAndLoadProperty1();

    void loadExceptions();
    void compileAndLoadExceptions();

    void loadScriptRef();
    void compileAndLoadScriptRef();

    void loadDefaultPropertyAlias();
    void compileAndLoadDefaultPropertyAlias();

    void initTestCase();
    void cleanupTestCase();

private:
    QQmlComponent *compileAndLoad(QQmlEngine *engine, const QString &file, const QList<QString> &dependencies = QList<QString>());
    QQmlComponent *load(QQmlEngine *engine, const QString &file);
    void printErrors(const QList<QQmlError>& errors);

};

#endif // TESTSIMPLEQMLLOAD_H
