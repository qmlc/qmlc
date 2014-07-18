/*!
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 * Contact: http://www.qt-project.org/legal
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
 * In addition, as a special exception, Digia and other copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

#ifndef PROPERTYCACHECREATOR_H
#define PROPERTYCACHECREATOR_H

#include <QList>
#include <QHash>
#include <QVector>
#include <QByteArray>

#include <private/qqmlirbuilder_p.h>
#include <private/qqmlimport_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qqmlpropertycache_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qqmlengine_p.h>

class QmcTypeCompiler;

class PropertyCacheCreator
{
public:
    PropertyCacheCreator(QmcTypeCompiler *compiler);
    virtual ~PropertyCacheCreator();
    bool buildMetaObjects();
    bool buildMetaObjectRecursively(int objectIndex, int referencingObjectIndex, const QV4::CompiledData::Binding *instantiatingBinding);
    bool ensureMetaObject(int objectIndex);
    bool createMetaObject(int objectIndex, const QmlIR::Object *obj, QQmlPropertyCache *baseTypeCache);

private:
    QString stringAt(int idx) const;
    QmcTypeCompiler *compiler;
    const QList<QmlIR::Object*> &qmlObjects;
    const QQmlImports *imports;
    QHash<int, QQmlCompiledData::TypeReference*> *resolvedTypes;
    QVector<QByteArray> vmeMetaObjects;
    QVector<QQmlPropertyCache*> propertyCaches;
    QQmlEnginePrivate *enginePrivate;
};

#endif // PROPERTYCACHECREATOR_H
