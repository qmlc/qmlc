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

#ifndef JSCODEGENERATOR_H
#define JSCODEGENERATOR_H

#include <private/qqmlcompiler_p.h>

#include <QHash>
#include <QVector>

class QmcTypeCompiler;
class QQmlPropertyCache;
class QQmlCustomParser;

namespace QmlIR {
struct JSCodeGen;
}

class JSCodeGenerator
{
public:
    JSCodeGenerator(QmcTypeCompiler *typeCompiler, QmlIR::JSCodeGen *v4CodeGen);

    bool generateCodeForComponents();

private:
    bool compileComponent(int componentRoot, const QHash<int, int> &objectIndexToId);
    bool compileJavaScriptCodeInObjectsRecursively(int objectIndex, int scopeObjectIndex);

    bool isComponent(int objectIndex) const { return objectIndexToIdPerComponent.contains(objectIndex); }

    QmcTypeCompiler *compiler;

    const QHash<int, QHash<int, int> > &objectIndexToIdPerComponent;
    const QHash<int, QQmlCompiledData::TypeReference*> &resolvedTypes;
    const QHash<int, QQmlCustomParser*> &customParsers;
    const QList<QmlIR::Object*> &qmlObjects;
    const QVector<QQmlPropertyCache *> &propertyCaches;
    QmlIR::JSCodeGen * const v4CodeGen;
};

#endif // JSCODEGENERATOR_H
