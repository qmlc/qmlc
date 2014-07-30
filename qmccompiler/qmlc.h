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
 *
 * Author: Mikko Hurskainen <mikko.hurskainen@nomovok.com>
 */

#ifndef QMLC_H
#define QMLC_H

#include <QObject>
#include <QUrl>
#include <QDataStream>
#include <QHash>

#include "compiler.h"
#include "qmccompiler_global.h"

class QQmlEngine;

class QMCCOMPILERSHARED_EXPORT QmlC : public Compiler
{
    Q_OBJECT

public:
    QmlC(QQmlEngine *engine, QObject *parent = 0);
    virtual ~QmlC();

protected:
    virtual bool compileData();
    virtual bool createExportStructures();

private:
    bool compileComponent(int recursion);
    bool dataReceived();
    bool continueLoadFromIR();
    bool resolveTypes();
    bool done();
    bool doCompile();
    bool loadImplicitImport();
    QmlCompilation* getComponent(const QUrl& url);

    bool implicitImportLoaded;
    static int MAX_RECURSION;
    int recursion;

    QHash<QString, QmlCompilation *> components;

    Q_DISABLE_COPY(QmlC)
};

#endif // QMLC_H
