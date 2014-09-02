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

#include <private/qqmlvmemetaobject_p.h>

#include "qmcunitpropertycachecreator.h"

static QAtomicInt classIndexCounter(0); // TBD: share with QQmlTypeLoader

QmcUnitPropertyCacheCreator::QmcUnitPropertyCacheCreator(QmcTypeUnit *qmcTypeUnit) :
    qmcTypeUnit(qmcTypeUnit),
    qmcUnit(qmcTypeUnit->qmcUnit()),
    enginePrivate(QQmlEnginePrivate::get(qmcTypeUnit->qmcUnit()->engine)),
    resolvedTypes(&qmcTypeUnit->compiledData->resolvedTypes)
{
}

QmcUnitPropertyCacheCreator::~QmcUnitPropertyCacheCreator()
{
}

QString QmcUnitPropertyCacheCreator::tr(QString string)
{
    return string;
}

void QmcUnitPropertyCacheCreator::recordError(const QV4::CompiledData::Location &location, const QString &description)
{
    QQmlError error;
    error.setLine(location.line);
    error.setColumn(location.column);
    error.setDescription(description);
    qmcUnit->errors.append(error);
}

quint32 QmcUnitPropertyCacheCreator::objectCount()
{
    return qmcUnit->qmlUnit->nObjects;
}

bool QmcUnitPropertyCacheCreator::buildMetaObjects()
{
    qmcTypeUnit->propertyCaches.resize(objectCount());
    qmcTypeUnit->vmeMetaObjects.resize(objectCount());

    if (!buildMetaObjectRecursively(qmcUnit->qmlUnit->indexOfRootObject, /*referencing object*/-1, /*instantiating binding*/0))
        return false;

    return true;
}

bool QmcUnitPropertyCacheCreator::buildMetaObjectRecursively(int objectIndex, int referencingObjectIndex, const QV4::CompiledData::Binding *instantiatingBinding)
{
    const QV4::CompiledData::Object *obj = qmcUnit->qmlUnit->objectAt(objectIndex);

    QQmlPropertyCache *baseTypeCache = 0;
    QQmlPropertyData *instantiatingProperty = 0;
    if (instantiatingBinding && instantiatingBinding->type == QV4::CompiledData::Binding::Type_GroupProperty) {
        Q_ASSERT(referencingObjectIndex >= 0);
        QQmlPropertyCache *parentCache = qmcTypeUnit->propertyCaches.at(referencingObjectIndex);
        Q_ASSERT(parentCache);
        Q_ASSERT(instantiatingBinding->propertyNameIndex != 0);

        bool notInRevision = false;
        instantiatingProperty = QmlIR::PropertyResolver(parentCache).property(qmcTypeUnit->stringAt(instantiatingBinding->propertyNameIndex), &notInRevision);
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

    bool needVMEMetaObject = obj->nProperties != 0 || obj->nSignals != 0 || obj->nFunctions != 0;
    if (!needVMEMetaObject) {
        const QV4::CompiledData::Binding *bindingTable = obj->bindingTable();
        for (unsigned int i = 0; i < obj->nBindings; i++) {
            const QV4::CompiledData::Binding* binding = &bindingTable[i];
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
            if (obj->nProperties > 0) {
                recordError(obj->location, tr("Fully dynamic types cannot declare new properties."));
                return false;
            }
            if (obj->nSignals > 0) {
                recordError(obj->location, tr("Fully dynamic types cannot declare new signals."));
                return false;
            }
            if (obj->nFunctions > 0) {
                recordError(obj->location, tr("Fully Dynamic types cannot declare new functions."));
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
            recordError(instantiatingBinding->location, tr("Non-existent attached object"));
            return false;
        }
        baseTypeCache = enginePrivate->cache(attachedMo);
        Q_ASSERT(baseTypeCache);
    }

    if (needVMEMetaObject) {
        if (!createMetaObject(objectIndex, obj, baseTypeCache))
            return false;
    } else if (baseTypeCache) {
        qmcTypeUnit->propertyCaches[objectIndex] = baseTypeCache;
        baseTypeCache->addref();
    }

    if (qmcTypeUnit->propertyCaches.at(objectIndex)) {
        const QV4::CompiledData::Binding *bindingTable = obj->bindingTable();
        for (unsigned int i = 0; i < obj->nBindings; i++) {
            const QV4::CompiledData::Binding* binding = &bindingTable[i];
            if (binding->type >= QV4::CompiledData::Binding::Type_Object) {
                if (!buildMetaObjectRecursively(binding->value.objectIndex, objectIndex, binding))
                    return false;
            }
        }
    }

    quint32 rootIndex = qmcTypeUnit->compiledData->qmlUnit->indexOfRootObject;
    Q_ASSERT(rootIndex < qmcTypeUnit->compiledData->qmlUnit->nObjects);
    qmcTypeUnit->compiledData->rootPropertyCache = qmcTypeUnit->propertyCaches[rootIndex];
    Q_ASSERT(qmcTypeUnit->compiledData->rootPropertyCache);
    qmcTypeUnit->compiledData->rootPropertyCache->addref();

    return true;
}

bool QmcUnitPropertyCacheCreator::ensureMetaObject(int objectIndex)
{
    if (!qmcTypeUnit->vmeMetaObjects.at(objectIndex).isEmpty())
        return true;
    const QV4::CompiledData::Object *obj = qmcUnit->qmlUnit->objectAt(objectIndex);
    QQmlCompiledData::TypeReference *typeRef = resolvedTypes->value(obj->inheritedTypeNameIndex);
    Q_ASSERT(typeRef);
    QQmlPropertyCache *baseTypeCache = typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
    return createMetaObject(objectIndex, obj, baseTypeCache);
}

bool QmcUnitPropertyCacheCreator::createMetaObject(int objectIndex, const QV4::CompiledData::Object *obj, QQmlPropertyCache *baseTypeCache)
{
    QQmlPropertyCache *cache = baseTypeCache->copyAndReserve(QQmlEnginePrivate::get(enginePrivate),
                                                             obj->nProperties,
                                                             obj->nFunctions + obj->nProperties + obj->nSignals,
                                                             obj->nSignals + obj->nProperties);
    qmcTypeUnit->propertyCaches[objectIndex] = cache;

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

    if (objectIndex == (int)qmcUnit->qmlUnit->indexOfRootObject) {
        QString path = qmcUnit->url.path();
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

    const QV4::CompiledData::Property* propertyTable = obj->propertyTable();
    for (unsigned int i = 0; i < obj->nProperties; i++) {
        const QV4::CompiledData::Property* p = &propertyTable[i];
        if (p->type == QV4::CompiledData::Property::Alias)
            aliasCount++;
        else if (p->type == QV4::CompiledData::Property::Var)
            varPropCount++;

        // No point doing this for both the alias and non alias cases
        bool notInRevision = false;
        QQmlPropertyData *d = resolver.property(qmcTypeUnit->stringAt(p->nameIndex), &notInRevision);
        if (d && d->isFinal()) {
            recordError(p->location, tr("Cannot override FINAL property"));
            return false;
        }
    }

    typedef QQmlVMEMetaData VMD;

    QByteArray &dynamicData = qmcTypeUnit->vmeMetaObjects[objectIndex] = QByteArray(sizeof(QQmlVMEMetaData)
                                                              + obj->nProperties * sizeof(VMD::PropertyData)
                                                              + obj->nFunctions * sizeof(VMD::MethodData)
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

        for (unsigned int i = 0; i < obj->nProperties; i++) {
            const QV4::CompiledData::Property* p = &propertyTable[i];
            if ((ii == NSS_Normal && (p->type == QV4::CompiledData::Property::Alias ||
                                      p->type == QV4::CompiledData::Property::Var)) ||
                ((ii == NSS_Var) && (p->type != QV4::CompiledData::Property::Var)) ||
                ((ii == NSS_Alias) && (p->type != QV4::CompiledData::Property::Alias)))
                continue;

            quint32 flags = QQmlPropertyData::IsSignal | QQmlPropertyData::IsFunction |
                            QQmlPropertyData::IsVMESignal;

            QString changedSigName = qmcTypeUnit->stringAt(p->nameIndex) + QLatin1String("Changed");
            seenSignals.insert(changedSigName);

            cache->appendSignal(changedSigName, flags, effectiveMethodIndex++);
        }
    }

    // Dynamic signals
    for (unsigned int i = 0; i < obj->nSignals; i++) {
        const QV4::CompiledData::Signal* s = obj->signalAt(i);
        const int paramCount = s->nParameters;

        QList<QByteArray> names;
        QVarLengthArray<int, 10> paramTypes(paramCount?(paramCount + 1):0);

        if (paramCount) {
            paramTypes[0] = paramCount;

            for (int i = 0; i < paramCount; ++i) {
                const QV4::CompiledData::Parameter *param = s->parameterAt(i);
                names.append(qmcTypeUnit->stringAt(param->nameIndex).toUtf8());
                if (param->type < builtinTypeCount) {
                    // built-in type
                    paramTypes[i + 1] = builtinTypes[param->type].metaType;
                } else {
                    // lazily resolved type
                    Q_ASSERT(param->type == QV4::CompiledData::Property::Custom);
                    QQmlCompiledData *ddata = qmcTypeUnit->refCompiledData();
                    Q_ASSERT(ddata->resolvedTypes.contains(param->customTypeNameIndex));
                    QQmlCompiledData::TypeReference *typeRef = ddata->resolvedTypes[param->customTypeNameIndex];
                    if (typeRef->component) { // isComposite = true
                        QQmlCompiledData *data = typeRef->component;
                        paramTypes[i + 1] = data->metaTypeId;
                    } else {
                        QQmlType *qmltype = typeRef->type;
                        paramTypes[i + 1] = qmltype->typeId();
                    }
                    ddata->release();
                }
            }
        }

        ((QQmlVMEMetaData *)dynamicData.data())->signalCount++;

        quint32 flags = QQmlPropertyData::IsSignal | QQmlPropertyData::IsFunction |
                        QQmlPropertyData::IsVMESignal;
        if (paramCount)
            flags |= QQmlPropertyData::HasArguments;

        QString signalName = qmcTypeUnit->stringAt(s->nameIndex);
        if (seenSignals.contains(signalName)) {
            recordError(s->location, tr("Duplicate signal name: invalid override of property change signal or superclass signal"));
            return false;
        }
        seenSignals.insert(signalName);

        cache->appendSignal(signalName, flags, effectiveMethodIndex++,
                            paramCount?paramTypes.constData():0, names);
    }


    // Dynamic slots
    const quint32 *functionIdx = obj->functionOffsetTable();
    for (unsigned int i = 0; i < obj->nFunctions; i++, functionIdx++) {
        const QV4::Function *s = qmcTypeUnit->compiledData->compilationUnit->runtimeFunctions[*functionIdx];
        // TBD: resolve through offset
        //QQmlJS::AST::FunctionDeclaration *astFunction = NULL; // s->functionDeclaration;

        quint32 flags = QQmlPropertyData::IsFunction | QQmlPropertyData::IsVMEFunction;

        if (s->compiledFunction->nFormals > 0)
            flags |= QQmlPropertyData::HasArguments;

        //QString slotName = astFunction->name.toString();
        QString slotName = qmcTypeUnit->stringAt(s->compiledFunction->nameIndex);
        if (seenSignals.contains(slotName)) {
            recordError(obj->location, tr("Duplicate method name: invalid override of property change signal or superclass signal"));
            return false;
        }
        // Note: we don't append slotName to the seenSignals list, since we don't
        // protect against overriding change signals or methods with properties.

        QList<QByteArray> parameterNames;
        //QQmlJS::AST::FormalParameterList *param = astFunction->formals;
        const quint32 *formalsIndices = s->compiledFunction->formalsTable();
        for (uint j = 0; j < s->compiledFunction->nFormals; j++) {
            QString name; s->compilationUnit->runtimeStrings[formalsIndices[j]].asString()->toQString();
            //parameterNames << param->name.toUtf8();
            parameterNames << name.toUtf8();
        }

        cache->appendMethod(slotName, flags, effectiveMethodIndex++, parameterNames);
    }


    // Dynamic properties (except var and aliases)
    int effectiveSignalIndex = cache->signalHandlerIndexCacheStart;
    int propertyIdx = 0;
    for (; propertyIdx < (int)obj->nProperties; ++propertyIdx) {
        const QV4::CompiledData::Property* p = &propertyTable[propertyIdx];
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

            // composite type usage, uses compiled table instead of import cache
            QQmlCompiledData *ddata = qmcTypeUnit->refCompiledData();
            Q_ASSERT(ddata->resolvedTypes.contains(p->customTypeNameIndex));
            QQmlCompiledData::TypeReference *typeRef = ddata->resolvedTypes[p->customTypeNameIndex];
            if (typeRef->component) { // isComposite = true
                QQmlCompiledData *data = typeRef->component;
                if (p->type == QV4::CompiledData::Property::Custom) {
                    propertyType = data->metaTypeId;
                    vmePropertyType = QMetaType::QObjectStar;
                } else {
                    propertyType = data->listMetaTypeId;
                    vmePropertyType = qMetaTypeId<QQmlListProperty<QObject> >();
                }
            } else {
                QQmlType *qmltype = typeRef->type;
                if (p->type == QV4::CompiledData::Property::Custom) {
                    propertyType = qmltype->typeId();
                    vmePropertyType = QMetaType::QObjectStar;
                } else {
                    propertyType = qmltype->qListTypeId();
                    vmePropertyType = qMetaTypeId<QQmlListProperty<QObject> >();
                }
            }
            ddata->release();

            if (p->type == QV4::CompiledData::Property::Custom)
                propertyFlags |= QQmlPropertyData::IsQObjectDerived;
            else
                propertyFlags |= QQmlPropertyData::IsQList;
        }

        if ((!p->flags & QV4::CompiledData::Property::IsReadOnly) && p->type != QV4::CompiledData::Property::CustomList)
            propertyFlags |= QQmlPropertyData::IsWritable;


        QString propertyName = qmcTypeUnit->stringAt(p->nameIndex);
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
    for (; propertyIdx < (int)obj->nProperties; ++propertyIdx) {
        const QV4::CompiledData::Property* p = &propertyTable[propertyIdx];

        if (p->type != QV4::CompiledData::Property::Var)
            continue;

        quint32 propertyFlags = QQmlPropertyData::IsVarProperty;
        if (!p->flags & QV4::CompiledData::Property::IsReadOnly)
            propertyFlags |= QQmlPropertyData::IsWritable;

        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        (vmd->propertyData() + vmd->propertyCount)->propertyType = QMetaType::QVariant;
        vmd->propertyCount++;
        ((QQmlVMEMetaData *)dynamicData.data())->varPropertyCount++;

        QString propertyName = qmcTypeUnit->stringAt(p->nameIndex);
        if (propertyIdx == obj->indexOfDefaultProperty) cache->_defaultPropertyName = propertyName;
        cache->appendProperty(propertyName, propertyFlags, effectivePropertyIndex++,
                              QMetaType::QVariant, effectiveSignalIndex);

        effectiveSignalIndex++;
    }

    // Alias property count.  Actual data is setup in buildDynamicMetaAliases
    ((QQmlVMEMetaData *)dynamicData.data())->aliasCount = aliasCount;
    // Dynamic slot data - comes after the property data
    for (uint i = 0; i < obj->nFunctions; i++) {
        const QV4::CompiledData::Function* function = qmcUnit->unit->functionAt(i);

        VMD::MethodData methodData = { /* runtimeFunctionIndex*/ 0, // ###
                                       /* formalsCount*/ (int)function->nFormals,
                                       /* s->location.start.line */0 }; // ###

        VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
        VMD::MethodData &md = *(vmd->methodData() + vmd->methodCount);
        vmd->methodCount++;
        md = methodData;
    }
    return true;
}
