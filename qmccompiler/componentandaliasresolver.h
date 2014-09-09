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

#ifndef COMPONENTANDALIASRESOLVER_H
#define COMPONENTANDALIASRESOLVER_H

#include <QVector>
#include <QHash>

#include <private/qqmljsmemorypool_p.h>
#include <private/qqmlcompiler_p.h>

class QmcTypeCompiler;
class QQmlEnginePrivate;
class QQmlPropertyCache;

namespace QmlIR {
class Object;
}

class ComponentAndAliasResolver
{
public:
    ComponentAndAliasResolver(QmcTypeCompiler *typeCompiler);

    bool resolve();

    QHash<int, int> getIdToObjectIndex();

protected:
    void findAndRegisterImplicitComponents(const QmlIR::Object *obj, QQmlPropertyCache *propertyCache);
    bool collectIdsAndAliases(int objectIndex);
    bool resolveAliases();
    QString stringAt(int idx) const;
    void recordError(const QV4::CompiledData::Location &location, const QString &description);

    QmcTypeCompiler *compiler;

    QQmlEnginePrivate *enginePrivate;
    QQmlJS::MemoryPool *pool;

    QList<QmlIR::Object*> *qmlObjects;
    const int indexOfRootObject;

    // indices of the objects that are actually Component {}
    QVector<int> componentRoots;
    // indices of objects that are the beginning of a new component
    // scope. This is sorted and used for binary search.
    QVector<quint32> componentBoundaries;

    int _componentIndex;
    QHash<int, int> _idToObjectIndex;
    QHash<int, int> *_objectIndexToIdInScope;
    QList<int> _objectsWithAliases;

    QHash<int, QQmlCompiledData::TypeReference*> *resolvedTypes;
    QVector<QQmlPropertyCache *> propertyCaches;
    QVector<QByteArray> *vmeMetaObjectData;
    QHash<int, int> *objectIndexToIdForRoot;
    QHash<int, QHash<int, int> > *objectIndexToIdPerComponent;
};

#endif // COMPONENTANDALIASRESOLVER_H
