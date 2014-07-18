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

#include "signalhandlerfunctionconverter.h"
#include "qmctypecompiler.h"
#include "qmlcompilation.h"

#define tr(x) QString(x)
#define COMPILE_EXCEPTION(binding, message) do { compiler->recordError(binding->location, message); } while(0)

SignalHandlerFunctionConverter::SignalHandlerFunctionConverter(QmcTypeCompiler *typeCompiler)
    : compiler(typeCompiler),
      qmlObjects(*typeCompiler->qmlObjects()),
      customParsers(typeCompiler->customParserCache()),
      resolvedTypes(*typeCompiler->resolvedTypes()),
      illegalNames(QV8Engine::get(typeCompiler->data()->engine)->illegalNames()),
      propertyCaches(typeCompiler->propertyCaches())
{
}

QString SignalHandlerFunctionConverter::stringAt(int idx) const
{
    return compiler->stringAt(idx);
}

bool SignalHandlerFunctionConverter::convertSignalHandlerExpressionsToFunctionDeclarations()
{
    for (int objectIndex = 0; objectIndex < qmlObjects.count(); ++objectIndex) {
        const QmlIR::Object * const obj = qmlObjects.at(objectIndex);
        QQmlPropertyCache *cache = propertyCaches.at(objectIndex);
        if (!cache)
            continue;
        if (QQmlCustomParser *customParser = customParsers.value(obj->inheritedTypeNameIndex)) {
            if (!(customParser->flags() & QQmlCustomParser::AcceptsSignalHandlers))
                continue;
        }
        const QString elementName = stringAt(obj->inheritedTypeNameIndex);
        if (!convertSignalHandlerExpressionsToFunctionDeclarations(obj, elementName, cache))
            return false;
    }
    return true;
}

bool SignalHandlerFunctionConverter::convertSignalHandlerExpressionsToFunctionDeclarations(const QmlIR::Object *obj, const QString &typeName, QQmlPropertyCache *propertyCache)
{
    // map from signal name defined in qml itself to list of parameters
    QHash<QString, QStringList> customSignals;

    for (QmlIR::Binding *binding = obj->firstBinding(); binding; binding = binding->next) {
        QString propertyName = stringAt(binding->propertyNameIndex);
        // Attached property?
        if (binding->type == QV4::CompiledData::Binding::Type_AttachedProperty) {
            const QmlIR::Object *attachedObj = qmlObjects.at(binding->value.objectIndex);
            QQmlCompiledData::TypeReference *typeRef = resolvedTypes.value(binding->propertyNameIndex);
            QQmlType *type = typeRef ? typeRef->type : 0;
            const QMetaObject *attachedType = type ? type->attachedPropertiesType() : 0;
            if (!attachedType)
                COMPILE_EXCEPTION(binding, tr("Non-existent attached object"));
            QQmlPropertyCache *cache = QQmlEnginePrivate::get(compiler->data()->engine)->cache(attachedType);
            if (!convertSignalHandlerExpressionsToFunctionDeclarations(attachedObj, propertyName, cache))
                return false;
            continue;
        }

        if (!QmlIR::IRBuilder::isSignalPropertyName(propertyName))
            continue;

        QmlIR::PropertyResolver resolver(propertyCache);

        Q_ASSERT(propertyName.startsWith(QStringLiteral("on")));
        propertyName.remove(0, 2);

        // Note that the property name could start with any alpha or '_' or '$' character,
        // so we need to do the lower-casing of the first alpha character.
        for (int firstAlphaIndex = 0; firstAlphaIndex < propertyName.size(); ++firstAlphaIndex) {
            if (propertyName.at(firstAlphaIndex).isUpper()) {
                propertyName[firstAlphaIndex] = propertyName.at(firstAlphaIndex).toLower();
                break;
            }
        }

        QList<QString> parameters;

        bool notInRevision = false;
        QQmlPropertyData *signal = resolver.signal(propertyName, &notInRevision);
        if (signal) {
            int sigIndex = propertyCache->methodIndexToSignalIndex(signal->coreIndex);
            sigIndex = propertyCache->originalClone(sigIndex);

            bool unnamedParameter = false;

            QList<QByteArray> parameterNames = propertyCache->signalParameterNames(sigIndex);
            for (int i = 0; i < parameterNames.count(); ++i) {
                const QString param = QString::fromUtf8(parameterNames.at(i));
                if (param.isEmpty())
                    unnamedParameter = true;
                else if (unnamedParameter) {
                    COMPILE_EXCEPTION(binding, tr("Signal uses unnamed parameter followed by named parameter."));
                } else if (illegalNames.contains(param)) {
                    COMPILE_EXCEPTION(binding, tr("Signal parameter \"%1\" hides global variable.").arg(param));
                }
                parameters += param;
            }
        } else {
            if (notInRevision) {
                // Try assinging it as a property later
                if (resolver.property(propertyName, /*notInRevision ptr*/0))
                    continue;

                const QString &originalPropertyName = stringAt(binding->propertyNameIndex);

                QQmlCompiledData::TypeReference *typeRef = resolvedTypes.value(obj->inheritedTypeNameIndex);
                const QQmlType *type = typeRef ? typeRef->type : 0;
                if (type) {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available in %3 %4.%5.").arg(typeName).arg(originalPropertyName).arg(type->module()).arg(type->majorVersion()).arg(type->minorVersion()));
                } else {
                    COMPILE_EXCEPTION(binding, tr("\"%1.%2\" is not available due to component versioning.").arg(typeName).arg(originalPropertyName));
                }
            }

            // Try to look up the signal parameter names in the object itself

            // build cache if necessary
            if (customSignals.isEmpty()) {
                for (const QmlIR::Signal *signal = obj->firstSignal(); signal; signal = signal->next) {
                    const QString &signalName = stringAt(signal->nameIndex);
                    customSignals.insert(signalName, signal->parameterStringList(compiler->stringPool()));
                }

                for (const QmlIR::Property *property = obj->firstProperty(); property; property = property->next) {
                    const QString propName = stringAt(property->nameIndex);
                    customSignals.insert(propName, QStringList());
                }
            }

            QHash<QString, QStringList>::ConstIterator entry = customSignals.find(propertyName);
            if (entry == customSignals.constEnd() && propertyName.endsWith(QStringLiteral("Changed"))) {
                QString alternateName = propertyName.mid(0, propertyName.length() - static_cast<int>(strlen("Changed")));
                entry = customSignals.find(alternateName);
            }

            if (entry == customSignals.constEnd()) {
                // Can't find even a custom signal, then just don't do anything and try
                // keeping the binding as a regular property assignment.
                continue;
            }

            parameters = entry.value();
        }

        // Binding object to signal means connect the signal to the object's default method.
        if (binding->type == QV4::CompiledData::Binding::Type_Object) {
            binding->flags |= QV4::CompiledData::Binding::IsSignalHandlerObject;
            continue;
        }

        if (binding->type != QV4::CompiledData::Binding::Type_Script) {
            if (binding->type < QV4::CompiledData::Binding::Type_Script) {
                COMPILE_EXCEPTION(binding, tr("Cannot assign a value to a signal (expecting a script to be run)"));
            } else {
                COMPILE_EXCEPTION(binding, tr("Incorrectly specified signal assignment"));
            }
        }

        QQmlJS::MemoryPool *pool = compiler->memoryPool();

        QQmlJS::AST::FormalParameterList *paramList = 0;
        foreach (const QString &param, parameters) {
            QStringRef paramNameRef = compiler->newStringRef(param);

            if (paramList)
                paramList = new (pool) QQmlJS::AST::FormalParameterList(paramList, paramNameRef);
            else
                paramList = new (pool) QQmlJS::AST::FormalParameterList(paramNameRef);
        }

        if (paramList)
            paramList = paramList->finish();

        QmlIR::CompiledFunctionOrExpression *foe = obj->functionsAndExpressions->slowAt(binding->value.compiledScriptIndex);
        QQmlJS::AST::Statement *statement = static_cast<QQmlJS::AST::Statement*>(foe->node);
        QQmlJS::AST::SourceElement *sourceElement = new (pool) QQmlJS::AST::StatementSourceElement(statement);
        QQmlJS::AST::SourceElements *elements = new (pool) QQmlJS::AST::SourceElements(sourceElement);
        elements = elements->finish();

        QQmlJS::AST::FunctionBody *body = new (pool) QQmlJS::AST::FunctionBody(elements);

        QQmlJS::AST::FunctionDeclaration *functionDeclaration = new (pool) QQmlJS::AST::FunctionDeclaration(compiler->newStringRef(stringAt(binding->propertyNameIndex)), paramList, body);
        functionDeclaration->functionToken = foe->node->firstSourceLocation();

        foe->node = functionDeclaration;
        binding->propertyNameIndex = compiler->registerString(propertyName);
        binding->flags |= QV4::CompiledData::Binding::IsSignalHandlerExpression;
    }
    return true;
}
