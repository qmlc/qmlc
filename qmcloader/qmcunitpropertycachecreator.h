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
 *
 * Author: Mikko Hurskainen <mikko.hurskainen@nomovok.com>
 */

#ifndef QMCUNITPROPERTYCACHECREATOR_H
#define QMCUNITPROPERTYCACHECREATOR_H

#include <private/qv4compileddata_p.h>
#include <private/qqmlcompiler_p.h>

#include "qmcunit.h"
#include "qmctypeunit.h"

// based on QQmlPropertyCacheCreator, uses QV4::CompiledData::QmlUnit instead of QmlIR::Object
class QmcUnitPropertyCacheCreator
{
public:
    QmcUnitPropertyCacheCreator(QmcTypeUnit *qmcTypeUnit);
    virtual ~QmcUnitPropertyCacheCreator();
    bool buildMetaObjects();
private:
    bool buildMetaObjectRecursively(int objectIndex, int referencingObjectIndex, const QV4::CompiledData::Binding *instantiatingBinding);
    bool createMetaObject(int objectIndex, const QV4::CompiledData::Object *obj, QQmlPropertyCache *baseTypeCache);
    bool ensureMetaObject(int objectIndex);
    void recordError(const QV4::CompiledData::Location &location, const QString &description);
    QString tr(QString);
    quint32 objectCount();
    QmcTypeUnit *qmcTypeUnit;
    QmcUnit *qmcUnit;
    QQmlEnginePrivate *enginePrivate;
    QHash<int, QQmlCompiledData::TypeReference *> *resolvedTypes;
};


#endif // QMCUNITPROPERTYCACHECREATOR_H
