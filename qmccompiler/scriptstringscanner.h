/*!
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 * Contact: http://www.qt-project.org/legal
 *
 * This file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * In addition, as a special exception, Digia and other copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

#ifndef SCRIPTSTRINGSCANNER_H
#define SCRIPTSTRINGSCANNER_H

#include <QList>
#include <QVector>

namespace QmlIR {
class Object;
}

class QQmlPropertyCache;

class QmcTypeCompiler;

class ScriptStringScanner
{
public:
    ScriptStringScanner(QmcTypeCompiler *compiler);

    void scan();

private:
    QmcTypeCompiler *compiler;
    const QList<QmlIR::Object*> &qmlObjects;
    const QVector<QQmlPropertyCache *> propertyCaches;

};

#endif // SCRIPTSTRINGSCANNER_H
