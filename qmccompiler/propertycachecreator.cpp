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

#include <QAtomicInt>

#include <private/qqmlvmemetaobject_p.h>

#include "propertycachecreator.h"
#include "qmctypecompiler.h"
#include "qmlcompilation.h"

#define tr(x) QString(x)

static QAtomicInt classIndexCounter(0); // TBD: how to share with QQmlTypeLoader

PropertyCacheCreator::PropertyCacheCreator(QmcTypeCompiler *compiler)
    : compiler(compiler),
      qmlObjects(*compiler->qmlObjects()),
      imports(compiler->data()->importCache),
      resolvedTypes(compiler->resolvedTypes()),
      enginePrivate(QQmlEnginePrivate::get(compiler->data()->engine))
{
}

PropertyCacheCreator::~PropertyCacheCreator()
{
    for (int i = 0; i < propertyCaches.count(); ++i)
        if (QQmlPropertyCache *cache = propertyCaches.at(i))
            cache->release();
    propertyCaches.clear();
}

QString PropertyCacheCreator::stringAt(int idx) const
{
    return compiler->stringAt(idx);
}

bool PropertyCacheCreator::buildMetaObjects()
{
    // qqmltypecompiler.cpp:451
    propertyCaches.resize(qmlObjects.count());
    vmeMetaObjects.resize(qmlObjects.count());

    if (!buildMetaObjectRecursively(compiler->rootObjectIndex(), /*referencing object*/-1, /*instantiating binding*/0))
        return false;

    compiler->setVMEMetaObjects(vmeMetaObjects);
    compiler->setPropertyCaches(propertyCaches);
    propertyCaches.clear();

    return true;
}

bool PropertyCacheCreator::buildMetaObjectRecursively(int objectIndex, int referencingObjectIndex, const QV4::CompiledData::Binding *instantiatingBinding)
{
    const QmlIR::Object *obj = qmlObjects.at(objectIndex);

    QQmlPropertyCache *baseTypeCache = 0;
    QQmlPropertyData *instantiatingProperty = 0;
    if (instantiatingBinding && instantiatingBinding->type == QV4::CompiledData::Binding::Type_GroupProperty) {
        Q_ASSERT(referencingObjectIndex >= 0);
        QQmlPropertyCache *parentCache = propertyCaches.at(referencingObjectIndex);
        Q_ASSERT(parentCache);
        Q_ASSERT(instantiatingBinding->propertyNameIndex != 0);

        bool notInRevision = false;
        instantiatingProperty = QmlIR::PropertyResolver(parentCache).property(stringAt(instantiatingBinding->propertyNameIndex), &notInRevision);
        if (instantiatingProperty) {
            if (instantiatingProperty->isQObject()) {
                baseTypeCache = enginePrivate->rawPropertyCacheForType(instantiatingProperty->propType);
                Q_ASSERT(baseTypeCache);
            } else if (QQmlValueType *vt = QQmlValueTypeFactory::valueType(instantiatingProperty->propType)) {
                baseTypeCache = enginePrivate->cache(vt->metaObject());
                Q_ASSERT(baseTypeCache);
            }
        }
    }

    bool needVMEMetaObject = obj->propertyCount() != 0 || obj->signalCount() != 0 || obj->functionCount() != 0;
    if (!needVMEMetaObject) {
        for (const QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (binding->type == QV4::CompiledData::Binding::Type_Object && (binding->flags & QV4::CompiledData::Binding::IsOnAssignment)) {

                // On assignments are implemented using value interceptors, which require a VME meta object.
                needVMEMetaObject = true;

                // If the on assignment is inside a group property, we need to distinguish between QObject based
                // group properties and value type group properties. For the former the base type is derived from
                // the property that references us, for the latter we only need a meta-object on the referencing object
                // because interceptors can't go to the shared value type instances.
                if (instantiatingProperty && QQmlValueTypeFactory::isValueType(instantiatingProperty->propType)) {
                    needVMEMetaObject = false;
                    if (!ensureMetaObject(referencingObjectIndex))
                        return false;
                }
                break;
            }
        }
    }

    if (obj->inheritedTypeNameIndex != 0) {
        QQmlCompiledData::TypeReference *typeRef = resolvedTypes->value(obj->inheritedTypeNameIndex);
        Q_ASSERT(typeRef);

        if (typeRef->isFullyDynamicType) {
            if (obj->propertyCount() > 0) {
                compiler->recordError(obj->location, tr("Fully dynamic types cannot declare new properties."));
                return false;
            }
            if (obj->signalCount() > 0) {
                compiler->recordError(obj->location, tr("Fully dynamic types cannot declare new signals."));
                return false;
            }
            if (obj->functionCount() > 0) {
                compiler->recordError(obj->location, tr("Fully Dynamic types cannot declare new functions."));
                return false;
            }
        }

        baseTypeCache = typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
        Q_ASSERT(baseTypeCache);
    } else if (instantiatingBinding && instantiatingBinding->isAttachedProperty()) {
        QQmlCompiledData::TypeReference *typeRef = resolvedTypes->value(instantiatingBinding->propertyNameIndex);
        Q_ASSERT(typeRef);
        const QMetaObject *attachedMo = typeRef->type ? typeRef->type->attachedPropertiesType() : 0;
        if (!attachedMo) {
            compiler->recordError(instantiatingBinding->location, tr("Non-existent attached object"));
            return false;
        }
        baseTypeCache = enginePrivate->cache(attachedMo);
        Q_ASSERT(baseTypeCache);
    }

    if (needVMEMetaObject) {
        if (!createMetaObject(objectIndex, obj, baseTypeCache))
            return false;
    } else if (baseTypeCache) {
        propertyCaches[objectIndex] = baseTypeCache;
        baseTypeCache->addref();
    }

    if (propertyCaches.at(objectIndex)) {
        for (const QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next)
            if (binding->type >= QV4::CompiledData::Binding::Type_Object) {
                if (!buildMetaObjectRecursively(binding->value.objectIndex, objectIndex, binding))
                    return false;
            }
    }

    return true;
}

bool PropertyCacheCreator::ensureMetaObject(int objectIndex)
{
    if (!vmeMetaObjects.at(objectIndex).isEmpty())
        return true;
    const QmlIR::Object *obj = qmlObjects.at(objectIndex);
    QQmlCompiledData::TypeReference *typeRef = resolvedTypes->value(obj->inheritedTypeNameIndex);
    Q_ASSERT(typeRef);
    QQmlPropertyCache *baseTypeCache = typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
    return createMetaObject(objectIndex, obj, baseTypeCache);
}

bool PropertyCacheCreator::createMetaObject(int objectIndex, const QmlIR::Object *obj, QQmlPropertyCache *baseTypeCache)
{
    QQmlPropertyCache *cache = baseTypeCache->copyAndReserve(QQmlEnginePrivate::get(enginePrivate),
                                                             obj->propertyCount(),
                                                             obj->functionCount() + obj->propertyCount() + obj->signalCount(),
                                                             obj->signalCount() + obj->propertyCount());
    propertyCaches[objectIndex] = cache;

    struct TypeData {
        QV4::CompiledData::Property::Type dtype;
        int metaType;
    } builtinTypes[] = {
        { QV4::CompiledData::Property::Var, qMetaTypeId<QJSValue>() },
        { QV4::CompiledData::Property::Variant, QMetaType::QVariant },
        { QV4::CompiledData::Property::Int, QMetaType::Int },
        { QV4::CompiledData::Property::Bool, QMetaType::Bool },
        { QV4::CompiledData::Property::Real, QMetaType::Double },
        { QV4::CompiledData::Property::String, QMetaType::QString },
        { QV4::CompiledData::Property::Url, QMetaType::QUrl },
        { QV4::CompiledData::Property::Color, QMetaType::QColor },
        { QV4::CompiledData::Property::Font, QMetaType::QFont },
        { QV4::CompiledData::Property::Time, QMetaType::QTime },
        { QV4::CompiledData::Property::Date, QMetaType::QDate },
        { QV4::CompiledData::Property::DateTime, QMetaType::QDateTime },
        { QV4::CompiledData::Property::Rect, QMetaType::QRectF },
        { QV4::CompiledData::Property::Point, QMetaType::QPointF },
        { QV4::CompiledData::Property::Size, QMetaType::QSizeF },
        { QV4::CompiledData::Property::Vector2D, QMetaType::QVector2D },
        { QV4::CompiledData::Property::Vector3D, QMetaType::QVector3D },
        { QV4::CompiledData::Property::Vector4D, QMetaType::QVector4D },
        { QV4::CompiledData::Property::Matrix4x4, QMetaType::QMatrix4x4 },
        { QV4::CompiledData::Property::Quaternion, QMetaType::QQuaternion }
    };
    static const uint builtinTypeCount = sizeof(builtinTypes) / sizeof(TypeData);

    QByteArray newClassName;

    if (objectIndex == compiler->rootObjectIndex()) {
        QString path = compiler->data()->url.path();
        int lastSlash = path.lastIndexOf(QLatin1Char('/'));
        if (lastSlash > -1) {
            QString nameBase = path.mid(lastSlash + 1, path.length()-lastSlash-5);
            if (!nameBase.isEmpty() && nameBase.at(0).isUpper())
                newClassName = nameBase.toUtf8() + "_QMLTYPE_" +
                               QByteArray::number(classIndexCounter.fetchAndAddRelaxed(1));
        }
    }
    if (newClassName.isEmpty()) {
        newClassName = QQmlMetaObject(baseTypeCache).className();
        newClassName.append("_QML_");
        newClassName.append(QByteArray::number(classIndexCounter.fetchAndAddRelaxed(1)));
    }

    cache->_dynamicClassName = newClassName;

    int aliasCount = 0;
    int varPropCount = 0;

    QmlIR::PropertyResolver resolver(baseTypeCache);

    for (const QmlIR::Property *p = obj->firstProperty(); p; p = p->next) {
        if (p->type == QV4::CompiledData::Property::Alias)
            aliasCount++;
        else if (p->type == QV4::CompiledData::Property::Var)
            varPropCount++;

        // No point doing this for both the alias and non alias cases
        bool notInRevision = false;
        QQmlPropertyData *d = resolver.property(stringAt(p->nameIndex), &notInRevision);
        if (d && d->isFinal()) {
            compiler->recordError(p->location, tr("Cannot override FINAL property"));
            return false;
        }
    }

    typedef QQmlVMEMetaData VMD;

    QByteArray &dynamicData = vmeMetaObjects[objectIndex] = QByteArray(sizeof(QQmlVMEMetaData)
                                                              + obj->propertyCount() * sizeof(VMD::PropertyData)
                                                              + obj->functionCount() * sizeof(VMD::MethodData)
                                                              + aliasCount * sizeof(VMD::AliasData), 0);

    int effectivePropertyIndex = cache->propertyIndexCacheStart;
    int effectiveMethodIndex = cache->methodIndexCacheStart;

    // For property change signal override detection.
    // We prepopulate a set of signal names which already exist in the object,
    // and throw an error if there is a signal/method defined as an override.
    QSet<QString> seenSignals;
    seenSignals << QStringLiteral("destroyed") << QStringLiteral("parentChanged") << QStringLiteral("objectNameChanged");
    QQmlPropertyCache *parentCache = cache;
    while ((parentCache = parentCache->parent())) {
        if (int pSigCount = parentCache->signalCount()) {
            int pSigOffset = parentCache->signalOffset();
            for (int i = pSigOffset; i < pSigCount; ++i) {
                QQmlPropertyData *currPSig = parentCache->signal(i);
                // XXX TODO: find a better way to get signal name from the property data :-/
                for (QQmlPropertyCache::StringCache::ConstIterator iter = parentCache->stringCache.begin();
                        iter != parentCache->stringCache.end(); ++iter) {
                    if (currPSig == (*iter).second) {
                        seenSignals.insert(iter.key());
                        break;
                    }
                }
            }
        }
    }

    // First set up notify signals for properties - first normal, then var, then alias
    enum { NSS_Normal = 0, NSS_Var = 1, NSS_Alias = 2 };
    for (int ii = NSS_Normal; ii <= NSS_Alias; ++ii) { // 0 == normal, 1 == var, 2 == alias

        if (ii == NSS_Var && varPropCount == 0) continue;
        else if (ii == NSS_Alias && aliasCount == 0) continue;

        for (const QmlIR::Property *p = obj->firstProperty(); p; p = p->next) {
            if ((ii == NSS_Normal && (p->type == QV4::CompiledData::Property::Alias ||
                                      p->type == QV4::CompiledData::Property::Var)) ||
                ((ii == NSS_Var) && (p->type != QV4::CompiledData::Property::Var)) ||
                ((ii == NSS_Alias) && (p->type != QV4::CompiledData::Property::Alias)))
                continue;

            quint32 flags = QQmlPropertyData::IsSignal | QQmlPropertyData::IsFunction |
                            QQmlPropertyData::IsVMESignal;

            QString changedSigName = stringAt(p->nameIndex) + QLatin1String("Changed");
            seenSignals.insert(changedSigName);

            cache->appendSignal(changedSigName, flags, effectiveMethodIndex++);
        }
    }

    // Dynamic signals
    for (const QmlIR::Signal *s = obj->firstSignal(); s; s = s->next) {
        const int paramCount = s->parameters->count;

        QList<QByteArray> names;
        QVarLengthArray<int, 10> paramTypes(paramCount?(paramCount + 1):0);

        if (paramCount) {
            paramTypes[0] = paramCount;

            QmlIR::SignalParameter *param = s->parameters->first;
            for (int i = 0; i < paramCount; ++i, param = param->next) {
                names.append(stringAt(param->nameIndex).toUtf8());
                if (param->type < builtinTypeCount) {
                    // built-in type
                    paramTypes[i + 1] = builtinTypes[param->type].metaType;
                } else {
                    // lazily resolved type
                    Q_ASSERT(param->type == QV4::CompiledData::Property::Custom);
                    const QString customTypeName = stringAt(param->customTypeNameIndex);
                    QQmlType *qmltype = 0;
                    if (!imports->resolveType(customTypeName, &qmltype, 0, 0, 0)) {
                        compiler->recordError(s->location, tr("Invalid signal parameter type: %1").arg(customTypeName));
                        return false;
                    }

                    if (qmltype->isComposite()) {
                        // TBD: how to handle composite type
                        qDebug() << "Composite type not supported";
#if 0
                        QQmlTypeData *tdata = NULL; //enginePrivate->typeLoader.getType(qmltype->sourceUrl());
                        Q_ASSERT(tdata);
                        Q_ASSERT(tdata->isComplete());

                        QQmlCompiledData *data = tdata->compiledData();

                        paramTypes[i + 1] = data->metaTypeId;

                        tdata->release();
#endif
                        return false;
                    } else {
                        paramTypes[i + 1] = qmltype->typeId();
                    }
                }
            }
        }

        ((QQmlVMEMetaData *)dynamicData.data())->signalCount++;

        quint32 flags = QQmlPropertyData::IsSignal | QQmlPropertyData::IsFunction |
                        QQmlPropertyData::IsVMESignal;
        if (paramCount)
            flags |= QQmlPropertyData::HasArguments;

        QString signalName = stringAt(s->nameIndex);
        if (seenSignals.contains(signalName)) {
            compiler->recordError(s->location, tr("Duplicate signal name: invalid override of property change signal or superclass signal"));
            return false;
        }
        seenSignals.insert(signalName);

        cache->appendSignal(signalName, flags, effectiveMethodIndex++,
                            paramCount?paramTypes.constData():0, names);
    }


    // Dynamic slots
    for (const QmlIR::Function *s = obj->firstFunction(); s; s = s->next) {
        QQmlJS::AST::FunctionDeclaration *astFunction = s->functionDeclaration;

        quint32 flags = QQmlPropertyData::IsFunction | QQmlPropertyData::IsVMEFunction;

        if (astFunction->formals)
            flags |= QQmlPropertyData::HasArguments;

        QString slotName = astFunction->name.toString();
        if (seenSignals.contains(slotName)) {
            compiler->recordError(s->location, tr("Duplicate method name: invalid override of property change signal or superclass signal"));
            return false;
        }

        // Note: we don't append slotName to the seenSignals list, since we don't
        // protect against overriding change signals or methods with properties.

        QList<QByteArray> parameterNames;
        QQmlJS::AST::FormalParameterList *param = astFunction->formals;
        while (param) {
            parameterNames << param->name.toUtf8();
            param = param->next;
        }

        cache->appendMethod(slotName, flags, effectiveMethodIndex++, parameterNames);
    }


    // Dynamic properties (except var and aliases)
    int effectiveSignalIndex = cache->signalHandlerIndexCacheStart;
    int propertyIdx = 0;
    for (const QmlIR::Property *p = obj->firstProperty(); p; p = p->next, ++propertyIdx) {

        if (p->type == QV4::CompiledData::Property::Alias ||
            p->type == QV4::CompiledData::Property::Var)
            continue;

        int propertyType = 0;
        int vmePropertyType = 0;
        quint32 propertyFlags = 0;

        if (p->type < builtinTypeCount) {
            propertyType = builtinTypes[p->type].metaType;
            vmePropertyType = propertyType;

            if (p->type == QV4::CompiledData::Property::Variant)
                propertyFlags |= QQmlPropertyData::IsQVariant;
        } else {
            Q_ASSERT(p->type == QV4::CompiledData::Property::CustomList ||
                     p->type == QV4::CompiledData::Property::Custom);

            QQmlType *qmltype = 0;
            if (!imports->resolveType(stringAt(p->customTypeNameIndex), &qmltype, 0, 0, 0)) {
                compiler->recordError(p->location, tr("Invalid property type"));
                return false;
            }

            Q_ASSERT(qmltype);
            if (qmltype->isComposite()) {
                qDebug() << "Composite type not supported";
                QQmlTypeData *tdata = NULL; //enginePrivate->typeLoader.getType(qmltype->sourceUrl());
                Q_ASSERT(tdata);
                Q_ASSERT(tdata->isComplete());

                QQmlCompiledData *data = tdata->compiledData();

                if (p->type == QV4::CompiledData::Property::Custom) {
                    propertyType = data->metaTypeId;
                    vmePropertyType = QMetaType::QObjectStar;
                } else {
                    propertyType = data->listMetaTypeId;
                    vmePropertyType = qMetaTypeId<QQmlListProperty<QObject> >();
                }

                tdata->release();
            } else {
                if (p->type == QV4::CompiledData::Property::Custom) {
                    propertyType = qmltype->typeId();
                    vmePropertyType = QMetaType::QObjectStar;
                } else {
                    propertyType = qmltype->qListTypeId();
                    vmePropertyType = qMetaTypeId<QQmlListProperty<QObject> >();
                }
            }

            if (p->type == QV4::CompiledData::Property::Custom)
                propertyFlags |= QQmlPropertyData::IsQObjectDerived;
            else
                propertyFlags |= QQmlPropertyData::IsQList;
        }

        if ((!p->flags & QV4::CompiledData::Property::IsReadOnly) && p->type != QV4::CompiledData::Property::CustomList)
            propertyFlags |= QQmlPropertyData::IsWritable;


        QString propertyName = stringAt(p->nameIndex);
        if (propertyIdx == obj->indexOfDefaultProperty) cache->_defaultPropertyName = propertyName;
        cache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                              propertyType, effectiveSignalIndex);

        effectiveSignalIndex++;

        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        (vmd->propertyData() + vmd->propertyCount)->propertyType = vmePropertyType;
        vmd->propertyCount++;
    }

    // Now do var properties
    propertyIdx = 0;
    for (const QmlIR::Property *p = obj->firstProperty(); p; p = p->next, ++propertyIdx) {

        if (p->type != QV4::CompiledData::Property::Var)
            continue;

        quint32 propertyFlags = QQmlPropertyData::IsVarProperty;
        if (!p->flags & QV4::CompiledData::Property::IsReadOnly)
            propertyFlags |= QQmlPropertyData::IsWritable;

        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        (vmd->propertyData() + vmd->propertyCount)->propertyType = QMetaType::QVariant;
        vmd->propertyCount++;
        ((QQmlVMEMetaData *)dynamicData.data())->varPropertyCount++;

        QString propertyName = stringAt(p->nameIndex);
        if (propertyIdx == obj->indexOfDefaultProperty) cache->_defaultPropertyName = propertyName;
        cache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                              QMetaType::QVariant, effectiveSignalIndex);

        effectiveSignalIndex++;
    }

    // Alias property count.  Actual data is setup in buildDynamicMetaAliases
    ((QQmlVMEMetaData *)dynamicData.data())->aliasCount = aliasCount;

    // Dynamic slot data - comes after the property data
    for (const QmlIR::Function *s = obj->firstFunction(); s; s = s->next) {
        QQmlJS::AST::FunctionDeclaration *astFunction = s->functionDeclaration;
        int formalsCount = 0;
        QQmlJS::AST::FormalParameterList *param = astFunction->formals;
        while (param) {
            formalsCount++;
            param = param->next;
        }

        VMD::MethodData methodData = { /* runtimeFunctionIndex*/ 0, // ###
                                       formalsCount,
                                       /* s->location.start.line */0 }; // ###

        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        VMD::MethodData &md = *(vmd->methodData() + vmd->methodCount);
        vmd->methodCount++;
        md = methodData;
    }

    return true;
}
