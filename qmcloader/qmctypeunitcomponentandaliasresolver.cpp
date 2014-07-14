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

#include <QByteArray>

#include <private/qqmlvmemetaobject_p.h>

#include "qmctypeunitcomponentandaliasresolver.h"
#include "qmcfile.h"
#include "qmctypeunit.h"
#include "qmcunit.h"

QmcTypeUnitComponentAndAliasResolver::QmcTypeUnitComponentAndAliasResolver(QmcTypeUnit *qmcTypeUnit)
    : qmcTypeUnit(qmcTypeUnit)
{
}

bool QmcTypeUnitComponentAndAliasResolver::resolve()
{
    // TBD: component related stuff
    // foreach (component) -> addAliases
    foreach (const QmcUnitObjectIndexToIdComponent &mapping, qmcTypeUnit->qmcUnit()->objectIndexToIdComponent) {
       QHash<int,int> &map = qmcTypeUnit->compiledData->objectIndexToIdPerComponent[mapping.componentIndex];
       foreach (const QmcUnitObjectIndexToId &componentMapping, mapping.mappings) {
            if (!addMapping(componentMapping, map))
                return false;
       }
    }

    // add mappings for root object
    foreach (const QmcUnitObjectIndexToId &mapping, qmcTypeUnit->qmcUnit()->objectIndexToIdRoot) {
        if (!addMapping(mapping, qmcTypeUnit->compiledData->objectIndexToIdForRoot))
            return false;
    }

    // add aliases
    if (!addAliases())
        return false;

    return true;
}

bool QmcTypeUnitComponentAndAliasResolver::addMapping(const QmcUnitObjectIndexToId& mapping, QHash<int, int> &mapTable)
{
    // add mappings
    mapTable.insert(mapping.index, mapping.id);

    // TBD call recursively based on bindings
    // qqmltypecompiler.cpp:1551
    return true;
}

bool QmcTypeUnitComponentAndAliasResolver::addAliases()
{
    // add aliases
    //QmcUnitAlias al = {0, 0, 0, 8, 0, 0, 24};
    //qmcTypeUnit->unit->aliases.append(al);
    // see QQmlComponentAndAliasResolver::resolve
    int effectiveAliasIndex = -1;
    int effectivePropertyIndex = -1;
    int effectiveSignalIndex = -1;
    int currentObjectIndex = -1;
    QQmlPropertyCache *propertyCache = NULL;
    foreach (const QmcUnitAlias &alias, qmcTypeUnit->unit->aliases) {
        if ((int)alias.objectIndex != currentObjectIndex) {
            effectiveAliasIndex = 0;
            propertyCache = qmcTypeUnit->propertyCaches.at(alias.objectIndex);
            effectivePropertyIndex = propertyCache->propertyIndexCacheStart + propertyCache->propertyIndexCache.count();
            effectiveSignalIndex = propertyCache->signalHandlerIndexCacheStart + propertyCache->propertyIndexCache.count();
        }
        Q_ASSERT(propertyCache);
        QQmlVMEMetaData::AliasData aliasData;
        aliasData.contextIdx = alias.contextIndex;
        aliasData.propertyIdx = alias.targetPropertyIndex;
        aliasData.propType = alias.propertyType;
        aliasData.flags = alias.flags;
        aliasData.notifySignal = alias.notifySignal;

        typedef QQmlVMEMetaData VMD;
        QByteArray &dynamicData = qmcTypeUnit->vmeMetaObjects[alias.objectIndex];
        Q_ASSERT(!dynamicData.isEmpty());
        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        *(vmd->aliasData() + effectiveAliasIndex++) = aliasData;

        Q_ASSERT(dynamicData.isDetached());

        // TBD: propertyCache
        const QV4::CompiledData::Object *obj = qmcTypeUnit->compiledData->qmlUnit->objectAt(alias.objectIndex);
        Q_ASSERT(obj);
        Q_ASSERT(alias.propertyIndex < obj->nProperties);
        const QV4::CompiledData::Property *p = &obj->propertyTable()[alias.propertyIndex];
        Q_ASSERT(p);
        QString propertyName = qmcTypeUnit->stringAt(p->nameIndex);
        const QString aliasPropertyValue = qmcTypeUnit->stringAt(p->aliasPropertyValueIndex);

        //const int idIndex = p->aliasIdValueIndex;
        const int targetObjectIndex = alias.objectIndex;
#if 0
        const int idIndex = p->aliasIdValueIndex;
        const int targetObjectIndex = _idToObjectIndex.value(idIndex, -1);
        if (targetObjectIndex == -1) {
            recordError(p->aliasLocation, tr("Invalid alias reference. Unable to find id \"%1\"").arg(stringAt(idIndex)));
            return false;
        }
#endif
        QStringRef property;
        QStringRef subProperty;

        const int propertySeparator = aliasPropertyValue.indexOf(QLatin1Char('.'));
        if (propertySeparator != -1) {
            property = aliasPropertyValue.leftRef(propertySeparator);
            subProperty = aliasPropertyValue.midRef(propertySeparator + 1);
        } else
            property = QStringRef(&aliasPropertyValue, 0, aliasPropertyValue.length());

        quint32 propertyFlags = QQmlPropertyData::IsAlias;
        int type = 0;
        bool writable = false;
        bool resettable = false;

        if (property.isEmpty()) {
            const QV4::CompiledData::Object *targetObject = qmcTypeUnit->compiledData->qmlUnit->objectAt(targetObjectIndex);
            QQmlCompiledData::TypeReference *typeRef = qmcTypeUnit->compiledData->resolvedTypes.value(targetObject->inheritedTypeNameIndex);
            Q_ASSERT(typeRef);

            if (typeRef->type)
                type = typeRef->type->typeId();
            else
                type = typeRef->component->metaTypeId;

            //flags |= QML_ALIAS_FLAG_PTR;
            propertyFlags |= QQmlPropertyData::IsQObjectDerived;
        } else {
            QQmlPropertyCache *targetCache = qmcTypeUnit->propertyCaches.at(targetObjectIndex);
            Q_ASSERT(targetCache);
            QmlIR::PropertyResolver resolver(targetCache);

            QQmlPropertyData *targetProperty = resolver.property(property.toString());
            if (!targetProperty || targetProperty->coreIndex > 0x0000FFFF) {
                return false;
            }

            //propIdx = targetProperty->coreIndex;
            type = targetProperty->propType;

            writable = targetProperty->isWritable();
            resettable = targetProperty->isResettable();
            //notifySignal = targetProperty->notifyIndex;

            if (!subProperty.isEmpty()) {
                QQmlValueType *valueType = QQmlValueTypeFactory::valueType(type);
                if (!valueType) {
                    return false;
                }

                //propType = type;

                int valueTypeIndex =
                    valueType->metaObject()->indexOfProperty(subProperty.toString().toUtf8().constData());
                if (valueTypeIndex == -1) {
                    return false;
                }
                Q_ASSERT(valueTypeIndex <= 0x0000FFFF);

                //propIdx |= (valueTypeIndex << 16);
                if (valueType->metaObject()->property(valueTypeIndex).isEnumType())
                    type = QVariant::Int;
                else
                    type = valueType->metaObject()->property(valueTypeIndex).userType();

            } else {
                if (targetProperty->isEnum()) {
                    type = QVariant::Int;
                } else {
                    // Copy type flags
                    propertyFlags |= targetProperty->getFlags() & QQmlPropertyData::PropTypeFlagMask;

                    if (targetProperty->isVarProperty())
                        propertyFlags |= QQmlPropertyData::IsQVariant;

#if 0
                    if (targetProperty->isQObject())
                        flags |= QML_ALIAS_FLAG_PTR;
#endif
                }
            }
        }

        if (!(p->flags & QV4::CompiledData::Property::IsReadOnly) && writable)
            propertyFlags |= QQmlPropertyData::IsWritable;
        else
            propertyFlags &= ~QQmlPropertyData::IsWritable;

        if (resettable)
            propertyFlags |= QQmlPropertyData::IsResettable;
        else
            propertyFlags &= ~QQmlPropertyData::IsResettable;

        if ((int)alias.propertyIndex == obj->indexOfDefaultProperty) propertyCache->defaultPropertyName() = propertyName;
        propertyCache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                                      type, effectiveSignalIndex++);
    }

    return true;
}
