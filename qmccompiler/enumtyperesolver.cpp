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

#include "enumtyperesolver.h"

#include "qmctypecompiler.h"
#include "qmlcompilation.h"

#define tr(x) QString(x)

EnumTypeResolver::EnumTypeResolver(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      qmlObjects(*typeCompiler->qmlObjects()),
      propertyCaches(typeCompiler->propertyCaches()),
      imports(typeCompiler->data()->importCache),
      resolvedTypes(typeCompiler->resolvedTypes())
{
}

bool EnumTypeResolver::resolveEnumBindings()
{
    for (int i = 0; i < qmlObjects.count(); ++i) {
        QQmlPropertyCache *propertyCache = propertyCaches.at(i);
        if (!propertyCache)
            continue;
        const QmlIR::Object *obj = qmlObjects.at(i);

        QmlIR::PropertyResolver resolver(propertyCache);

        for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
            if (binding->flags & QV4::CompiledData::Binding::IsSignalHandlerExpression
                || binding->flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
                continue;

            if (binding->type != QV4::CompiledData::Binding::Type_Script)
                continue;

            const QString propertyName = compiler->stringAt(binding->propertyNameIndex);
            bool notInRevision = false;
            QQmlPropertyData *pd = resolver.property(propertyName, &notInRevision);
            if (!pd)
                continue;

            if (!pd->isEnum() && pd->propType != QMetaType::Int)
                continue;

            if (!tryQualifiedEnumAssignment(obj, propertyCache, pd, binding))
                return false;
        }
    }

    return true;
}

struct StaticQtMetaObject : public QObject
{
    static const QMetaObject *get()
        { return &staticQtMetaObject; }
};

bool EnumTypeResolver::tryQualifiedEnumAssignment(const QmlIR::Object *obj, const QQmlPropertyCache *propertyCache, const QQmlPropertyData *prop, QmlIR::Binding *binding)
{
    bool isIntProp = (prop->propType == QMetaType::Int) && !prop->isEnum();
    if (!prop->isEnum() && !isIntProp)
        return true;

    if (!prop->isWritable() && !(binding->flags & QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration)) {
        compiler->recordError(binding->location, tr("Invalid property assignment: \"%1\" is a read-only property").arg(compiler->stringAt(binding->propertyNameIndex)));
        return false;
    }

    Q_ASSERT(binding->type = QV4::CompiledData::Binding::Type_Script);
    const QString string = compiler->bindingAsString(obj, binding->value.compiledScriptIndex);
    if (!string.constData()->isUpper())
        return true;

    int dot = string.indexOf(QLatin1Char('.'));
    if (dot == -1 || dot == string.length()-1)
        return true;

    if (string.indexOf(QLatin1Char('.'), dot+1) != -1)
        return true;

    QHashedStringRef typeName(string.constData(), dot);
    QString enumValue = string.mid(dot+1);

    if (isIntProp) {
        // Allow enum assignment to ints.
        bool ok;
        int enumval = evaluateEnum(typeName.toString(), enumValue.toUtf8(), &ok);
        if (ok) {
            binding->type = QV4::CompiledData::Binding::Type_Number;
            binding->value.d = (double)enumval;
            binding->flags |= QV4::CompiledData::Binding::IsResolvedEnum;
        }
        return true;
    }
    QQmlType *type = 0;
    imports->resolveType(typeName, &type, 0, 0, 0);

    if (!type && typeName != QLatin1String("Qt"))
        return true;
    if (type && type->isComposite()) //No enums on composite (or composite singleton) types
        return true;

    int value = 0;
    bool ok = false;

    QQmlCompiledData::TypeReference *tr = resolvedTypes->value(obj->inheritedTypeNameIndex);
    if (type && tr && tr->type == type) {
        QMetaProperty mprop = propertyCache->firstCppMetaObject()->property(prop->coreIndex);

        // When these two match, we can short cut the search
        if (mprop.isFlagType()) {
            value = mprop.enumerator().keysToValue(enumValue.toUtf8().constData(), &ok);
        } else {
            value = mprop.enumerator().keyToValue(enumValue.toUtf8().constData(), &ok);
        }
    } else {
        // Otherwise we have to search the whole type
        if (type) {
            value = type->enumValue(QHashedStringRef(enumValue), &ok);
        } else {
            QByteArray enumName = enumValue.toUtf8();
            const QMetaObject *metaObject = StaticQtMetaObject::get();
            for (int ii = metaObject->enumeratorCount() - 1; !ok && ii >= 0; --ii) {
                QMetaEnum e = metaObject->enumerator(ii);
                value = e.keyToValue(enumName.constData(), &ok);
            }
        }
    }

    if (!ok)
        return true;

    binding->type = QV4::CompiledData::Binding::Type_Number;
    binding->value.d = (double)value;
    binding->flags |= QV4::CompiledData::Binding::IsResolvedEnum;
    return true;
}

int EnumTypeResolver::evaluateEnum(const QString &scope, const QByteArray &enumValue, bool *ok) const
{
    Q_ASSERT_X(ok, "QQmlEnumTypeResolver::evaluateEnum", "ok must not be a null pointer");
    *ok = false;

    if (scope != QLatin1String("Qt")) {
        QQmlType *type = 0;
        imports->resolveType(scope, &type, 0, 0, 0);
        return type ? type->enumValue(QHashedCStringRef(enumValue.constData(), enumValue.length()), ok) : -1;
    }

    const QMetaObject *mo = StaticQtMetaObject::get();
    int i = mo->enumeratorCount();
    while (i--) {
        int v = mo->enumerator(i).keyToValue(enumValue.constData(), ok);
        if (*ok)
            return v;
    }
    return -1;
}
