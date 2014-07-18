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

#ifndef ENUMTYPERESOLVER_H
#define ENUMTYPERESOLVER_H

#include <QString>
#include <QByteArray>

#include <private/qqmlcompiler_p.h>

class QmcTypeCompiler;
class QQmlPropertyCache;
class QQmlPropertyData;
class QQmlImports;

namespace QmlIR {
struct Object;
struct Binding;
}

class EnumTypeResolver
{
public:
    EnumTypeResolver(QmcTypeCompiler *typeCompiler);
    bool resolveEnumBindings();
private:
    bool tryQualifiedEnumAssignment(const QmlIR::Object *obj, const QQmlPropertyCache *propertyCache, const QQmlPropertyData *prop, QmlIR::Binding *binding);
    int evaluateEnum(const QString &scope, const QByteArray &enumValue, bool *ok) const;
    QmcTypeCompiler *compiler;

    const QList<QmlIR::Object*> &qmlObjects;
    const QVector<QQmlPropertyCache *> propertyCaches;
    const QQmlImports *imports;
    QHash<int, QQmlCompiledData::TypeReference *> *resolvedTypes;
};

#endif // ENUMTYPERESOLVER_H
