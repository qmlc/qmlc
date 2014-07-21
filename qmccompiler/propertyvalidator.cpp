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

#include <private/qqmlimport_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qqmlcustomparser_p.h>
#include <private/qqmlpropertycache_p.h>
#include <private/qqmlengine_p.h>
#include <private/qqmlstringconverters_p.h>

#include "propertyvalidator.h"

#include "qmctypecompiler.h"
#include "qmlcompilation.h"

#define tr(x) QString(x)
#define COMPILE_EXCEPTION(o, x) do { recordError(o->location, x); return false; } while(0)

PropertyValidator::PropertyValidator(QmcTypeCompiler* typeCompiler)
    : compiler(typeCompiler),
      enginePrivate(QQmlEnginePrivate::get(typeCompiler->data()->engine)),
      qmlUnit(typeCompiler->data()->compiledData->qmlUnit),
      resolvedTypes(*typeCompiler->resolvedTypes()),
      customParsers(typeCompiler->customParserCache()),
      propertyCaches(typeCompiler->propertyCaches()),
      objectIndexToIdPerComponent(*typeCompiler->objectIndexToIdPerComponent()),
      customParserData(typeCompiler->customParserData()),
      _seenObjectWithId(false)
{
}

bool PropertyValidator::validate()
{
    if (!validateObject(qmlUnit->indexOfRootObject, /*instantiatingBinding*/0))
        return false;
    compiler->setCustomParserBindings(customParserBindings);
    compiler->setDeferredBindingsPerObject(deferredBindingsPerObject);
    return true;
}

const QQmlImports &PropertyValidator::imports() const
{
    return *compiler->data()->importCache;
}

QQmlBinding::Identifier PropertyValidator::bindingIdentifier(const QV4::CompiledData::Binding *binding, QQmlCustomParser *)
{
    const int id = customParserBindings.count();
    customParserBindings.append(binding->value.compiledScriptIndex);
    return id;
}

QString PropertyValidator::bindingAsString(int objectIndex, const QV4::CompiledData::Binding *binding) const
{
    const QmlIR::Object *object = compiler->qmlObjects()->value(objectIndex);
    if (!object)
        return QString();
    int reverseIndex = object->runtimeFunctionIndices->indexOf(binding->value.compiledScriptIndex);
    if (reverseIndex == -1)
        return QString();
    return compiler->bindingAsString(object, reverseIndex);
}

typedef QVarLengthArray<const QV4::CompiledData::Binding *, 8> GroupPropertyVector;

struct BindingFinder
{
    bool operator()(quint32 name, const QV4::CompiledData::Binding *binding) const
    {
        return name < binding->propertyNameIndex;
    }
    bool operator()(const QV4::CompiledData::Binding *binding, quint32 name) const
    {
        return binding->propertyNameIndex < name;
    }
    bool operator()(const QV4::CompiledData::Binding *lhs, const QV4::CompiledData::Binding *rhs) const
    {
        return lhs->propertyNameIndex < rhs->propertyNameIndex;
    }
};

bool PropertyValidator::validateObject(int objectIndex, const QV4::CompiledData::Binding *instantiatingBinding, bool populatingValueTypeGroupProperty)
{
    const QV4::CompiledData::Object *obj = qmlUnit->objectAt(objectIndex);
    if (obj->idIndex != 0)
        _seenObjectWithId = true;

    if (isComponent(objectIndex)) {
        Q_ASSERT(obj->nBindings == 1);
        const QV4::CompiledData::Binding *componentBinding = obj->bindingTable();
        Q_ASSERT(componentBinding->type == QV4::CompiledData::Binding::Type_Object);
        return validateObject(componentBinding->value.objectIndex, componentBinding);
    }

    QQmlPropertyCache *propertyCache = propertyCaches.at(objectIndex);
    if (!propertyCache)
        return true;

    QStringList deferredPropertyNames;
    {
        const QMetaObject *mo = propertyCache->firstCppMetaObject();
        const int namesIndex = mo->indexOfClassInfo("DeferredPropertyNames");
        if (namesIndex != -1) {
            QMetaClassInfo classInfo = mo->classInfo(namesIndex);
            deferredPropertyNames = QString::fromUtf8(classInfo.value()).split(QLatin1Char(','));
        }
    }

    QQmlCustomParser *customParser = customParsers.value(obj->inheritedTypeNameIndex);
    QList<const QV4::CompiledData::Binding*> customBindings;

    // Collect group properties first for sanity checking
    // vector values are sorted by property name string index.
    GroupPropertyVector groupProperties;
    const QV4::CompiledData::Binding *binding = obj->bindingTable();
    for (quint32 i = 0; i < obj->nBindings; ++i, ++binding) {
        if (!binding->isGroupProperty())
                continue;

        if (binding->flags & QV4::CompiledData::Binding::IsOnAssignment)
            continue;

        if (populatingValueTypeGroupProperty) {
            recordError(binding->location, tr("Property assignment expected"));
            return false;
        }

        GroupPropertyVector::const_iterator pos = std::lower_bound(groupProperties.constBegin(), groupProperties.constEnd(), binding->propertyNameIndex, BindingFinder());
        groupProperties.insert(pos, binding);
    }

    QBitArray customParserBindings(obj->nBindings);
    QBitArray deferredBindings;

    QmlIR::PropertyResolver propertyResolver(propertyCache);

    QString defaultPropertyName;
    QQmlPropertyData *defaultProperty = 0;
    if (obj->indexOfDefaultProperty != -1) {
        QQmlPropertyCache *cache = propertyCache->parent();
        defaultPropertyName = cache->defaultPropertyName();
        defaultProperty = cache->defaultProperty();
    } else {
        defaultPropertyName = propertyCache->defaultPropertyName();
        defaultProperty = propertyCache->defaultProperty();
    }

    binding = obj->bindingTable();
    for (quint32 i = 0; i < obj->nBindings; ++i, ++binding) {
        QString name = stringAt(binding->propertyNameIndex);

        if (customParser) {
            if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
                if (customParser->flags() & QQmlCustomParser::AcceptsAttachedProperties) {
                    customBindings << binding;
                    customParserBindings.setBit(i);
                    continue;
                }
            } else if (QmlIR::IRBuilder::isSignalPropertyName(name)
                       && !(customParser->flags() & QQmlCustomParser::AcceptsSignalHandlers)) {
                customBindings << binding;
                customParserBindings.setBit(i);
                continue;
            }
        }

        // Signal handlers were resolved and checked earlier in the signal handler conversion pass.
        if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression
            || binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
            continue;

        if (name.constData()->isUpper() && !binding->isAttachedProperty()) {
            QQmlType *type = 0;
            QQmlImportNamespace *typeNamespace = 0;
            imports().resolveType(stringAt(binding->propertyNameIndex), &type, 0, 0, &typeNamespace);
            if (typeNamespace)
                recordError(binding->location, tr("Invalid use of namespace"));
            else
                recordError(binding->location, tr("Invalid attached object assignment"));
            return false;
        }

        bool bindingToDefaultProperty = false;

        bool notInRevision = false;
        QQmlPropertyData *pd = 0;
        if (!name.isEmpty()) {
            if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression
                || binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
                pd = propertyResolver.signal(name, &notInRevision);
            else
                pd = propertyResolver.property(name, &notInRevision);

            if (notInRevision) {
                QString typeName = stringAt(obj->inheritedTypeNameIndex);
                QQmlCompiledData::TypeReference *objectType = resolvedTypes.value(obj->inheritedTypeNameIndex);
                if (objectType && objectType->type) {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available in %3 %4.%5.").arg(typeName).arg(name).arg(objectType->type->module()).arg(objectType->majorVersion).arg(objectType->minorVersion));
                } else {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available due to component versioning.").arg(typeName).arg(name));
                }
            }
        } else {
           if (instantiatingBinding && instantiatingBinding->type == QV4::CompiledData::Binding::Type_GroupProperty)
               COMPILE_EXCEPTION(binding, tr("Cannot assign a value directly to a grouped property"));

           pd = defaultProperty;
           name = defaultPropertyName;
           bindingToDefaultProperty = true;
        }

        bool seenSubObjectWithId = false;

        if (binding->type >= QV4::CompiledData::Binding::Type_Object && !customParser) {
            qSwap(_seenObjectWithId, seenSubObjectWithId);
            const bool subObjectValid = validateObject(binding->value.objectIndex, binding, pd && QQmlValueTypeFactory::valueType(pd->propType));
            qSwap(_seenObjectWithId, seenSubObjectWithId);
            if (!subObjectValid)
                return false;
            _seenObjectWithId |= seenSubObjectWithId;
        }

        if (!seenSubObjectWithId
            && !deferredPropertyNames.isEmpty() && deferredPropertyNames.contains(name)) {

            if (deferredBindings.isEmpty())
                deferredBindings.resize(obj->nBindings);

            deferredBindings.setBit(i);
        }

        if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
            if (instantiatingBinding && (instantiatingBinding->isAttachedProperty() || instantiatingBinding->isGroupProperty())) {
                recordError(binding->location, tr("Attached properties cannot be used here"));
                return false;
            }
            continue;
        }

        if (pd) {
            GroupPropertyVector::const_iterator assignedGroupProperty = std::lower_bound(groupProperties.constBegin(), groupProperties.constEnd(), binding->propertyNameIndex, BindingFinder());
            const bool assigningToGroupProperty = assignedGroupProperty != groupProperties.constEnd() && !(binding->propertyNameIndex < (*assignedGroupProperty)->propertyNameIndex);

            if (!pd->isWritable()
                && !pd->isQList()
                && !binding->isGroupProperty()
                && !(binding->flags & QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration)
                ) {

                if (assigningToGroupProperty && binding->type < QV4::CompiledData::Binding::Type_Object)
                    recordError(binding->valueLocation, tr("Cannot assign a value directly to a grouped property"));
                else
                    recordError(binding->valueLocation, tr("Invalid property assignment: \"%1\" is a read-only property").arg(name));
                return false;
            }

            if (!pd->isQList() && (binding->flags & QV4::CompiledData::Binding::IsListItem)) {
                QString error;
                if (pd->propType == qMetaTypeId<QQmlScriptString>())
                    error = tr( "Cannot assign multiple values to a script property");
                else
                    error = tr( "Cannot assign multiple values to a singular property");
                recordError(binding->valueLocation, error);
                return false;
            }

            if (!bindingToDefaultProperty
                && !binding->isGroupProperty()
                && !(binding->flags & QV4::CompiledData::Binding::IsOnAssignment)
                && assigningToGroupProperty) {
                QV4::CompiledData::Location loc = binding->valueLocation;
                if (loc < (*assignedGroupProperty)->valueLocation)
                    loc = (*assignedGroupProperty)->valueLocation;

                if (pd && QQmlValueTypeFactory::isValueType(pd->propType))
                    recordError(loc, tr("Property has already been assigned a value"));
                else
                    recordError(loc, tr("Cannot assign a value directly to a grouped property"));
                return false;
            }

            if (binding->type < QV4::CompiledData::Binding::Type_Script) {
                if (!validateLiteralBinding(propertyCache, pd, binding))
                    return false;
            } else if (binding->type == QV4::CompiledData::Binding::Type_Object) {
                if (!validateObjectBinding(pd, name, binding))
                    return false;
            } else if (binding->isGroupProperty()) {
                if (QQmlValueTypeFactory::isValueType(pd->propType)) {
                    if (QQmlValueTypeFactory::valueType(pd->propType)) {
                        if (!pd->isWritable()) {
                            recordError(binding->location, tr("Invalid property assignment: \"%1\" is a read-only property").arg(name));
                            return false;
                        }
                    } else {
                        recordError(binding->location, tr("Invalid grouped property access"));
                        return false;
                    }
                } else {
                    if (!enginePrivate->propertyCacheForType(pd->propType)) {
                        recordError(binding->location, tr("Invalid grouped property access"));
                        return false;
                    }
                }
            }
        } else {
            if (customParser) {
                customBindings << binding;
                customParserBindings.setBit(i);
                continue;
            }
            if (bindingToDefaultProperty) {
                COMPILE_EXCEPTION(binding, tr("Cannot assign to non-existent default property"));
            } else {
                COMPILE_EXCEPTION(binding, tr("Cannot assign to non-existent property \"%1\"").arg(name));
            }
        }
    }

    if (customParser && !customBindings.isEmpty()) {
        customParser->clearErrors();
        customParser->compiler = this;
        QQmlCompiledData::CustomParserData data;
        data.bindings = customParserBindings;
        data.compilationArtifact = customParser->compile(qmlUnit, customBindings);
        customParser->compiler = 0;
        customParserData->insert(objectIndex, data);
        const QList<QQmlError> parserErrors = customParser->errors();
        if (!parserErrors.isEmpty()) {
            foreach (QQmlError error, parserErrors)
                compiler->recordError(error);
            return false;
        }
    }

    if (!deferredBindings.isEmpty())
        deferredBindingsPerObject.insert(objectIndex, deferredBindings);

    return true;
}

bool PropertyValidator::validateLiteralBinding(QQmlPropertyCache *propertyCache, QQmlPropertyData *property, const QV4::CompiledData::Binding *binding)
{
    if (property->isQList()) {
        recordError(binding->valueLocation, tr("Cannot assign primitives to lists"));
        return false;
    }

    if (property->isEnum()) {
        if (binding->flags & QV4::CompiledData::Binding::IsResolvedEnum)
            return true;

        QString value = binding->valueAsString(&qmlUnit->header);
        QMetaProperty p = propertyCache->firstCppMetaObject()->property(property->coreIndex);
        bool ok;
        if (p.isFlagType()) {
            p.enumerator().keysToValue(value.toUtf8().constData(), &ok);
        } else
            p.enumerator().keyToValue(value.toUtf8().constData(), &ok);

        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: unknown enumeration"));
            return false;
        }
        return true;
    }

    switch (property->propType) {
    case QMetaType::QVariant:
    break;
    case QVariant::String: {
        if (!binding->evaluatesToString()) {
            recordError(binding->valueLocation, tr("Invalid property assignment: string expected"));
            return false;
        }
    }
    break;
    case QVariant::StringList: {
        if (!binding->evaluatesToString()) {
            recordError(binding->valueLocation, tr("Invalid property assignment: string or string list expected"));
            return false;
        }
    }
    break;
    case QVariant::ByteArray: {
        if (binding->type != QV4::CompiledData::Binding::Type_String) {
            recordError(binding->valueLocation, tr("Invalid property assignment: byte array expected"));
            return false;
        }
    }
    break;
    case QVariant::Url: {
        if (binding->type != QV4::CompiledData::Binding::Type_String) {
            recordError(binding->valueLocation, tr("Invalid property assignment: url expected"));
            return false;
        }
    }
    break;
    case QVariant::UInt: {
        if (binding->type == QV4::CompiledData::Binding::Type_Number) {
            double d = binding->valueAsNumber();
            if (double(uint(d)) == d)
                return true;
        }
        recordError(binding->valueLocation, tr("Invalid property assignment: unsigned int expected"));
        return false;
    }
    break;
    case QVariant::Int: {
        if (binding->type == QV4::CompiledData::Binding::Type_Number) {
            double d = binding->valueAsNumber();
            if (double(int(d)) == d)
                return true;
        }
        recordError(binding->valueLocation, tr("Invalid property assignment: int expected"));
        return false;
    }
    break;
    case QMetaType::Float: {
        if (binding->type != QV4::CompiledData::Binding::Type_Number) {
            recordError(binding->valueLocation, tr("Invalid property assignment: number expected"));
            return false;
        }
    }
    break;
    case QVariant::Double: {
        if (binding->type != QV4::CompiledData::Binding::Type_Number) {
            recordError(binding->valueLocation, tr("Invalid property assignment: number expected"));
            return false;
        }
    }
    break;
    case QVariant::Color: {
        bool ok = false;
        QQmlStringConverters::rgbaFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: color expected"));
            return false;
        }
    }
    break;
#ifndef QT_NO_DATESTRING
    case QVariant::Date: {
        bool ok = false;
        QQmlStringConverters::dateFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: date expected"));
            return false;
        }
    }
    break;
    case QVariant::Time: {
        bool ok = false;
        QQmlStringConverters::timeFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: time expected"));
            return false;
        }
    }
    break;
    case QVariant::DateTime: {
        bool ok = false;
        QQmlStringConverters::dateTimeFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: datetime expected"));
            return false;
        }
    }
    break;
#endif // QT_NO_DATESTRING
    case QVariant::Point: {
        bool ok = false;
        QQmlStringConverters::pointFFromString(binding->valueAsString(&qmlUnit->header), &ok).toPoint();
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: point expected"));
            return false;
        }
    }
    break;
    case QVariant::PointF: {
        bool ok = false;
        QQmlStringConverters::pointFFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: point expected"));
            return false;
        }
    }
    break;
    case QVariant::Size: {
        bool ok = false;
        QQmlStringConverters::sizeFFromString(binding->valueAsString(&qmlUnit->header), &ok).toSize();
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: size expected"));
            return false;
        }
    }
    break;
    case QVariant::SizeF: {
        bool ok = false;
        QQmlStringConverters::sizeFFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: size expected"));
            return false;
        }
    }
    break;
    case QVariant::Rect: {
        bool ok = false;
        QQmlStringConverters::rectFFromString(binding->valueAsString(&qmlUnit->header), &ok).toRect();
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: rect expected"));
            return false;
        }
    }
    break;
    case QVariant::RectF: {
        bool ok = false;
        QQmlStringConverters::rectFFromString(binding->valueAsString(&qmlUnit->header), &ok);
        if (!ok) {
            recordError(binding->valueLocation, tr("Invalid property assignment: point expected"));
            return false;
        }
    }
    break;
    case QVariant::Bool: {
        if (binding->type != QV4::CompiledData::Binding::Type_Boolean) {
            recordError(binding->valueLocation, tr("Invalid property assignment: boolean expected"));
            return false;
        }
    }
    break;
    case QVariant::Vector3D: {
        struct {
            float xp;
            float yp;
            float zy;
        } vec;
        if (!QQmlStringConverters::createFromString(QMetaType::QVector3D, binding->valueAsString(&qmlUnit->header), &vec, sizeof(vec))) {
            recordError(binding->valueLocation, tr("Invalid property assignment: 3D vector expected"));
            return false;
        }
    }
    break;
    case QVariant::Vector4D: {
        struct {
            float xp;
            float yp;
            float zy;
            float wp;
        } vec;
        if (!QQmlStringConverters::createFromString(QMetaType::QVector4D, binding->valueAsString(&qmlUnit->header), &vec, sizeof(vec))) {
            recordError(binding->valueLocation, tr("Invalid property assignment: 4D vector expected"));
            return false;
        }
    }
    break;
    case QVariant::RegExp:
        recordError(binding->valueLocation, tr("Invalid property assignment: regular expression expected; use /pattern/ syntax"));
        return false;
    default: {
        // generate single literal value assignment to a list property if required
        if (property->propType == qMetaTypeId<QList<qreal> >()) {
            if (binding->type != QV4::CompiledData::Binding::Type_Number) {
                recordError(binding->valueLocation, tr("Invalid property assignment: real or array of reals expected"));
                return false;
            }
            break;
        } else if (property->propType == qMetaTypeId<QList<int> >()) {
            bool ok = (binding->type == QV4::CompiledData::Binding::Type_Number);
            if (ok) {
                double n = binding->valueAsNumber();
                if (double(int(n)) != n)
                    ok = false;
            }
            if (!ok)
                recordError(binding->valueLocation, tr("Invalid property assignment: int or array of ints expected"));
            break;
        } else if (property->propType == qMetaTypeId<QList<bool> >()) {
            if (binding->type != QV4::CompiledData::Binding::Type_Boolean) {
                recordError(binding->valueLocation, tr("Invalid property assignment: bool or array of bools expected"));
                return false;
            }
            break;
        } else if (property->propType == qMetaTypeId<QList<QUrl> >()) {
            if (binding->type != QV4::CompiledData::Binding::Type_String) {
                recordError(binding->valueLocation, tr("Invalid property assignment: url or array of urls expected"));
                return false;
            }
            break;
        } else if (property->propType == qMetaTypeId<QList<QString> >()) {
            if (!binding->evaluatesToString()) {
                recordError(binding->valueLocation, tr("Invalid property assignment: string or array of strings expected"));
                return false;
            }
            break;
        } else if (property->propType == qMetaTypeId<QJSValue>()) {
            break;
        } else if (property->propType == qMetaTypeId<QQmlScriptString>()) {
            break;
        }

        // otherwise, try a custom type assignment
        QQmlMetaType::StringConverter converter = QQmlMetaType::customStringConverter(property->propType);
        if (!converter) {
            recordError(binding->valueLocation, tr("Invalid property assignment: unsupported type \"%1\"").arg(QString::fromLatin1(QMetaType::typeName(property->propType))));
            return false;
        }
    }
    break;
    }
    return true;
}

/*!
    Returns true if from can be assigned to a (QObject) property of type
    to.
*/
bool PropertyValidator::canCoerce(int to, QQmlPropertyCache *fromMo)
{
    QQmlPropertyCache *toMo = enginePrivate->rawPropertyCacheForType(to);

    while (fromMo) {
        if (fromMo == toMo)
            return true;
        fromMo = fromMo->parent();
    }
    return false;
}

bool PropertyValidator::validateObjectBinding(QQmlPropertyData *property, const QString &propertyName, const QV4::CompiledData::Binding *binding)
{
    if (binding->flags & QV4::CompiledData::Binding::IsOnAssignment) {
        Q_ASSERT(binding->type == QV4::CompiledData::Binding::Type_Object);

        bool isValueSource = false;
        bool isPropertyInterceptor = false;

        QQmlType *qmlType = 0;
        const QV4::CompiledData::Object *targetObject = qmlUnit->objectAt(binding->value.objectIndex);
        QQmlCompiledData::TypeReference *typeRef = resolvedTypes.value(targetObject->inheritedTypeNameIndex);
        if (typeRef) {
            QQmlPropertyCache *cache = typeRef->createPropertyCache(QQmlEnginePrivate::get(enginePrivate));
            const QMetaObject *mo = cache->firstCppMetaObject();
            while (mo && !qmlType) {
                qmlType = QQmlMetaType::qmlType(mo);
                mo = mo->superClass();
            }
            Q_ASSERT(qmlType);
        }

        if (qmlType) {
            isValueSource = qmlType->propertyValueSourceCast() != -1;
            isPropertyInterceptor = qmlType->propertyValueInterceptorCast() != -1;
        }

        if (!isValueSource && !isPropertyInterceptor) {
            recordError(binding->valueLocation, tr("\"%1\" cannot operate on \"%2\"").arg(stringAt(targetObject->inheritedTypeNameIndex)).arg(propertyName));
            return false;
        }

        return true;
    }
    if (isComponent(binding->value.objectIndex))
        return true;

    if (QQmlMetaType::isInterface(property->propType)) {
        // Can only check at instantiation time if the created sub-object successfully casts to the
        // target interface.
        return true;
    } else if (property->propType == QMetaType::QVariant) {
        // We can convert everything to QVariant :)
        return true;
    } else if (property->isQList()) {
        const int listType = enginePrivate->listType(property->propType);
        if (!QQmlMetaType::isInterface(listType)) {
            QQmlPropertyCache *source = propertyCaches.at(binding->value.objectIndex);
            if (!canCoerce(listType, source)) {
                recordError(binding->valueLocation, tr("Cannot assign object to list"));
                return false;
            }
        }
        return true;
    } else if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject && property->isFunction()) {
        return true;
    } else if (QQmlValueTypeFactory::isValueType(property->propType)) {
        recordError(binding->location, tr("Unexpected object assignment"));
        return false;
    } else if (property->propType == qMetaTypeId<QQmlScriptString>()) {
        recordError(binding->valueLocation, tr("Invalid property assignment: script expected"));
        return false;
    } else {
        // We want to raw metaObject here as the raw metaobject is the
        // actual property type before we applied any extensions that might
        // effect the properties on the type, but don't effect assignability
        QQmlPropertyCache *propertyMetaObject = enginePrivate->rawPropertyCacheForType(property->propType);

        // Will be true if the assgned type inherits propertyMetaObject
        bool isAssignable = false;
        // Determine isAssignable value
        if (propertyMetaObject) {
            QQmlPropertyCache *c = propertyCaches.at(binding->value.objectIndex);
            while (c && !isAssignable) {
                isAssignable |= c == propertyMetaObject;
                c = c->parent();
            }
        }

        if (!isAssignable) {
            recordError(binding->valueLocation, tr("Cannot assign object to property"));
            return false;
        }
    }
    return true;
}

QString PropertyValidator::stringAt(int idx) const
{
    return compiler->stringAt(idx);
}

void PropertyValidator::recordError(const QV4::CompiledData::Location &location, const QString &message)
{
    compiler->recordError(location, message);
}
