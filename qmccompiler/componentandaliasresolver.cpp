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

#include <QQmlComponent>

#include "componentandaliasresolver.h"

#include <private/qqmlirbuilder_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qqmlpropertycache_p.h>
#include <private/qqmlvmemetaobject_p.h>

#include "qmctypecompiler.h"
#include "qmlcompilation.h"

#define tr(x) QString(x)
#define COMPILE_EXCEPTION(o, x) do { recordError(o->location, x); return false; } while(0)

ComponentAndAliasResolver::ComponentAndAliasResolver(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      enginePrivate(QQmlEnginePrivate::get(typeCompiler->data()->engine)),
      pool(typeCompiler->memoryPool()),
      qmlObjects(typeCompiler->qmlObjects()),
      indexOfRootObject(typeCompiler->rootObjectIndex()),
      _componentIndex(-1),
      _objectIndexToIdInScope(0),
      resolvedTypes(typeCompiler->resolvedTypes()),
      propertyCaches(typeCompiler->propertyCaches()),
      vmeMetaObjectData(&typeCompiler->data()->compiledData->metaObjects),
      objectIndexToIdForRoot(typeCompiler->objectIndexToIdForRoot()),
      objectIndexToIdPerComponent(typeCompiler->objectIndexToIdPerComponent())
{
}

void ComponentAndAliasResolver::recordError(const QV4::CompiledData::Location &location, const QString &description)
{
    compiler->recordError(location, description);
}

QString ComponentAndAliasResolver::stringAt(int idx) const
{
    return compiler->stringAt(idx);
}

void ComponentAndAliasResolver::findAndRegisterImplicitComponents(const QmlIR::Object *obj, QQmlPropertyCache *propertyCache)
{
    QmlIR::PropertyResolver propertyResolver(propertyCache);

    QQmlPropertyData *defaultProperty = obj->indexOfDefaultProperty != -1 ? propertyCache->parent()->defaultProperty() : propertyCache->defaultProperty();

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type != QV4::CompiledData::Binding::Type_Object)
            continue;
        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
            continue;

        const QmlIR::Object *targetObject = qmlObjects->at(binding->value.objectIndex);
        QQmlCompiledData::TypeReference *tr = resolvedTypes->value(targetObject->inheritedTypeNameIndex);
        Q_ASSERT(tr);
        if (QQmlType *targetType = tr->type) {
            if (targetType->metaObject() == &QQmlComponent::staticMetaObject)
                continue;
        } else if (tr->component) {
            if (tr->component->rootPropertyCache->firstCppMetaObject() == &QQmlComponent::staticMetaObject)
                continue;
        }

        QQmlPropertyData *pd = 0;
        if (binding->propertyNameIndex != 0) {
            bool notInRevision = false;
            pd = propertyResolver.property(stringAt(binding->propertyNameIndex), &notInRevision);
        } else {
            pd = defaultProperty;
        }
        if (!pd || !pd->isQObject())
            continue;

        QQmlPropertyCache *pc = enginePrivate->rawPropertyCacheForType(pd->propType);
        const QMetaObject *mo = pc->firstCppMetaObject();
        while (mo) {
            if (mo == &QQmlComponent::staticMetaObject)
                break;
            mo = mo->superClass();
        }

        if (!mo)
            continue;

        static QQmlType *componentType = QQmlMetaType::qmlType(&QQmlComponent::staticMetaObject);
        Q_ASSERT(componentType);

        QmlIR::Object *syntheticComponent = pool->New<QmlIR::Object>();
        syntheticComponent->init(pool, compiler->registerString(QString::fromUtf8(componentType->typeName())), compiler->registerString(QString()));
        syntheticComponent->location = binding->valueLocation;

        if (!resolvedTypes->contains(syntheticComponent->inheritedTypeNameIndex)) {
            QQmlCompiledData::TypeReference *typeRef = new QQmlCompiledData::TypeReference;
            typeRef->type = componentType;
            typeRef->majorVersion = componentType->majorVersion();
            typeRef->minorVersion = componentType->minorVersion();
            resolvedTypes->insert(syntheticComponent->inheritedTypeNameIndex, typeRef);
        }

        qmlObjects->append(syntheticComponent);
        const int componentIndex = qmlObjects->count() - 1;
        // Keep property caches symmetric
        QQmlPropertyCache *componentCache = enginePrivate->cache(&QQmlComponent::staticMetaObject);
        componentCache->addref();
        propertyCaches.append(componentCache);

        QmlIR::Binding *syntheticBinding = pool->New<QmlIR::Binding>();
        *syntheticBinding = *binding;
        syntheticBinding->type = QV4::CompiledData::Binding::Type_Object;
        QString error = syntheticComponent->appendBinding(syntheticBinding, /*isListBinding*/false);
        Q_ASSERT(error.isEmpty());
        Q_UNUSED(error);

        binding->value.objectIndex = componentIndex;

        componentRoots.append(componentIndex);
        componentBoundaries.append(syntheticBinding->value.objectIndex);
    }
}

bool ComponentAndAliasResolver::resolve()
{
    // Detect real Component {} objects as well as implicitly defined components, such as
    //     someItemDelegate: Item {}
    // In the implicit case Item is surrounded by a synthetic Component {} because the property
    // on the left hand side is of QQmlComponent type.
    const int objCountWithoutSynthesizedComponents = qmlObjects->count();
    for (int i = 0; i < objCountWithoutSynthesizedComponents; ++i) {
        const QmlIR::Object *obj = qmlObjects->at(i);
        QQmlPropertyCache *cache = propertyCaches.at(i);
        if (obj->inheritedTypeNameIndex == 0 && !cache)
            continue;

        bool isExplicitComponent = false;

        if (obj->inheritedTypeNameIndex) {
            QQmlCompiledData::TypeReference *tref = resolvedTypes->value(obj->inheritedTypeNameIndex);
            Q_ASSERT(tref);
            if (tref->type && tref->type->metaObject() == &QQmlComponent::staticMetaObject)
                isExplicitComponent = true;
        }
        if (!isExplicitComponent) {
            if (cache)
                findAndRegisterImplicitComponents(obj, cache);
            continue;
        }

        componentRoots.append(i);

        if (obj->functionCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new functions."));
        if (obj->propertyCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new properties."));
        if (obj->signalCount() > 0)
            COMPILE_EXCEPTION(obj, tr("Component objects cannot declare new signals."));

        if (obj->bindingCount() == 0)
            COMPILE_EXCEPTION(obj, tr("Cannot create empty component specification"));

        const QmlIR::Binding *rootBinding = obj->firstBinding();

        for (const QmlIR::Binding *b = rootBinding; b; b = b->next) {
            if (b->propertyNameIndex != 0)
                COMPILE_EXCEPTION(rootBinding, tr("Component elements may not contain properties other than id"));
        }

        if (rootBinding->next || rootBinding->type != QV4::CompiledData::Binding::Type_Object)
            COMPILE_EXCEPTION(obj, tr("Invalid component body specification"));

        componentBoundaries.append(rootBinding->value.objectIndex);
    }

    std::sort(componentBoundaries.begin(), componentBoundaries.end());

    for (int i = 0; i < componentRoots.count(); ++i) {
        const QmlIR::Object *component  = qmlObjects->at(componentRoots.at(i));
        const QmlIR::Binding *rootBinding = component->firstBinding();

        _componentIndex = i;
        _idToObjectIndex.clear();

        _objectIndexToIdInScope = &(*objectIndexToIdPerComponent)[componentRoots.at(i)];

        _objectsWithAliases.clear();

        if (!collectIdsAndAliases(rootBinding->value.objectIndex))
            return false;

        if (!resolveAliases())
            return false;

        compiler->appendAliasIdToObjectIndexPerComponent(rootBinding->value.objectIndex, _idToObjectIndex);
    }


    // Collect ids and aliases for root
    _componentIndex = -1;
    _idToObjectIndex.clear();
    _objectIndexToIdInScope = objectIndexToIdForRoot;
    _objectsWithAliases.clear();

    collectIdsAndAliases(indexOfRootObject);

    resolveAliases();

    compiler->setAliasIdToObjectIndex(_idToObjectIndex);

    // Implicit component insertion may have added objects and thus we also need
    // to extend the symmetric propertyCaches.
    compiler->setPropertyCaches(propertyCaches);

    return true;
}

bool ComponentAndAliasResolver::collectIdsAndAliases(int objectIndex)
{
    const QmlIR::Object *obj = qmlObjects->at(objectIndex);

    if (obj->idIndex != 0) {
        if (_idToObjectIndex.contains(obj->idIndex)) {
            recordError(obj->locationOfIdProperty, tr("id is not unique"));
            return false;
        }
        _idToObjectIndex.insert(obj->idIndex, objectIndex);
        _objectIndexToIdInScope->insert(objectIndex, _objectIndexToIdInScope->count());
    }

    for (const QmlIR::Property *property = obj->firstProperty(); property; property = property->next) {
        if (property->type == QV4::CompiledData::Property::Alias) {
            _objectsWithAliases.append(objectIndex);
            break;
        }
    }

    for (const QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        if (binding->type != QV4::CompiledData::Binding::Type_Object
            && binding->type != QV4::CompiledData::Binding::Type_AttachedProperty
            && binding->type != QV4::CompiledData::Binding::Type_GroupProperty)
            continue;

        // Stop at Component boundary
        if (std::binary_search(componentBoundaries.constBegin(), componentBoundaries.constEnd(), binding->value.objectIndex))
            continue;

        if (!collectIdsAndAliases(binding->value.objectIndex))
            return false;
    }

    return true;
}

bool ComponentAndAliasResolver::resolveAliases()
{
    foreach (int objectIndex, _objectsWithAliases) {
        const QmlIR::Object *obj = qmlObjects->at(objectIndex);

        QQmlPropertyCache *propertyCache = propertyCaches.at(objectIndex);
        Q_ASSERT(propertyCache);

        int effectiveSignalIndex = propertyCache->signalHandlerIndexCacheStart + propertyCache->propertyIndexCache.count();
        int effectivePropertyIndex = propertyCache->propertyIndexCacheStart + propertyCache->propertyIndexCache.count();
        int effectiveAliasIndex = 0;

        const QmlIR::Property *p = obj->firstProperty();
        for (int propertyIndex = 0; propertyIndex < obj->propertyCount(); ++propertyIndex, p = p->next) {
            if (p->type != QV4::CompiledData::Property::Alias)
                continue;

            const int idIndex = p->aliasIdValueIndex;
            const int targetObjectIndex = _idToObjectIndex.value(idIndex, -1);
            if (targetObjectIndex == -1) {
                recordError(p->aliasLocation, tr("Invalid alias reference. Unable to find id \"%1\"").arg(stringAt(idIndex)));
                return false;
            }
            const int targetId = _objectIndexToIdInScope->value(targetObjectIndex, -1);
            Q_ASSERT(targetId != -1);

            const QString aliasPropertyValue = stringAt(p->aliasPropertyValueIndex);

            QStringRef property;
            QStringRef subProperty;

            const int propertySeparator = aliasPropertyValue.indexOf(QLatin1Char('.'));
            if (propertySeparator != -1) {
                property = aliasPropertyValue.leftRef(propertySeparator);
                subProperty = aliasPropertyValue.midRef(propertySeparator + 1);
            } else
                property = QStringRef(&aliasPropertyValue, 0, aliasPropertyValue.length());

            int propIdx = -1;
            int propType = 0;
            int notifySignal = -1;
            int flags = 0;
            int type = 0;
            bool writable = false;
            bool resettable = false;

            quint32 propertyFlags = QQmlPropertyData::IsAlias;

            if (property.isEmpty()) {
                const QmlIR::Object *targetObject = qmlObjects->at(targetObjectIndex);
                QQmlCompiledData::TypeReference *typeRef = resolvedTypes->value(targetObject->inheritedTypeNameIndex);
                Q_ASSERT(typeRef);

                if (typeRef->type)
                    type = typeRef->type->typeId();
                else
                    type = typeRef->component->metaTypeId;

                flags |= QML_ALIAS_FLAG_PTR;
                propertyFlags |= QQmlPropertyData::IsQObjectDerived;
            } else {
                QQmlPropertyCache *targetCache = propertyCaches.at(targetObjectIndex);
                Q_ASSERT(targetCache);
                QmlIR::PropertyResolver resolver(targetCache);

                QQmlPropertyData *targetProperty = resolver.property(property.toString());
                if (!targetProperty || targetProperty->coreIndex > 0x0000FFFF) {
                    recordError(p->aliasLocation, tr("Invalid alias location"));
                    return false;
                }

                propIdx = targetProperty->coreIndex;
                type = targetProperty->propType;

                writable = targetProperty->isWritable();
                resettable = targetProperty->isResettable();
                notifySignal = targetProperty->notifyIndex;

                if (!subProperty.isEmpty()) {
                    QQmlValueType *valueType = QQmlValueTypeFactory::valueType(type);
                    if (!valueType) {
                        recordError(p->aliasLocation, tr("Invalid alias location"));
                        return false;
                    }

                    propType = type;

                    int valueTypeIndex =
                        valueType->metaObject()->indexOfProperty(subProperty.toString().toUtf8().constData());
                    if (valueTypeIndex == -1) {
                        recordError(p->aliasLocation, tr("Invalid alias location"));
                        return false;
                    }
                    Q_ASSERT(valueTypeIndex <= 0x0000FFFF);

                    propIdx |= (valueTypeIndex << 16);
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

                        if (targetProperty->isQObject())
                            flags |= QML_ALIAS_FLAG_PTR;
                    }
                }
            }

            QQmlVMEMetaData::AliasData aliasData = { targetId, propIdx, propType, flags, notifySignal };

            typedef QQmlVMEMetaData VMD;
            QByteArray &dynamicData = (*vmeMetaObjectData)[objectIndex];
            Q_ASSERT(!dynamicData.isEmpty());
            VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
            *(vmd->aliasData() + effectiveAliasIndex++) = aliasData;

            Q_ASSERT(dynamicData.isDetached());

            if (!(p->flags & QV4::CompiledData::Property::IsReadOnly) && writable)
                propertyFlags |= QQmlPropertyData::IsWritable;
            else
                propertyFlags &= ~QQmlPropertyData::IsWritable;

            if (resettable)
                propertyFlags |= QQmlPropertyData::IsResettable;
            else
                propertyFlags &= ~QQmlPropertyData::IsResettable;

            QString propertyName = stringAt(p->nameIndex);
            if (propertyIndex == obj->indexOfDefaultProperty) propertyCache->_defaultPropertyName = propertyName;
            propertyCache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                                          type, effectiveSignalIndex++);

        }
    }
    return true;
}
