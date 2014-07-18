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

#ifndef CUSTOMPARSERSCRIPTINDEXER_H
#define CUSTOMPARSERSCRIPTINDEXER_H

#include <QHash>
#include <QList>

namespace QmlIR {
struct Object;
}

class QQmlCustomParser;
class QmcTypeCompiler;

class CustomParserScriptIndexer
{
public:
    CustomParserScriptIndexer(QmcTypeCompiler *typeCompiler);

    void annotateBindingsWithScriptStrings();

private:
    void scanObjectRecursively(int objectIndex, bool annotateScriptBindings = false);

    QmcTypeCompiler *compiler;
    const QList<QmlIR::Object*> &qmlObjects;
    const QHash<int, QQmlCustomParser*> &customParsers;

};

#endif // CUSTOMPARSERSCRIPTINDEXER_H
