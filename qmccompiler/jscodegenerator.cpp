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

#include <QHash>

#include <private/qqmlirbuilder_p.h>
#include <private/qqmljsmemorypool_p.h>
#include <private/qv4compileddata_p.h>

#include "jscodegenerator.h"

#include "qmctypecompiler.h"

JSCodeGenerator::JSCodeGenerator(QmcTypeCompiler *typeCompiler, QmlIR::JSCodeGen* v4CodeGen)
    : compiler(typeCompiler),
      objectIndexToIdPerComponent(*typeCompiler->objectIndexToIdPerComponent()),
      resolvedTypes(*typeCompiler->resolvedTypes()),
      customParsers(typeCompiler->customParserCache()),
      qmlObjects(*typeCompiler->qmlObjects()),
      propertyCaches(typeCompiler->propertyCaches()),
      v4CodeGen(v4CodeGen)
{
}

bool JSCodeGenerator::generateCodeForComponents()
{
    const QHash<int, QHash<int, int> > &objectIndexToIdPerComponent = *compiler->objectIndexToIdPerComponent();
    for (QHash<int, QHash<int, int> >::ConstIterator component = objectIndexToIdPerComponent.constBegin(), end = objectIndexToIdPerComponent.constEnd();
         component != end; ++component) {
        if (!compileComponent(component.key(), component.value()))
            return false;
    }

    return compileComponent(compiler->rootObjectIndex(), *compiler->objectIndexToIdForRoot());
}

bool JSCodeGenerator::compileComponent(int contextObject, const QHash<int, int> &objectIndexToId)
{
    if (isComponent(contextObject)) {
        const QmlIR::Object *component = qmlObjects.at(contextObject);
        Q_ASSERT(component->bindingCount() == 1);
        const QV4::CompiledData::Binding *componentBinding = component->firstBinding();
        Q_ASSERT(componentBinding->type == QV4::CompiledData::Binding::Type_Object);
        contextObject = componentBinding->value.objectIndex;
    }

    QmlIR::JSCodeGen::ObjectIdMapping idMapping;
    if (!objectIndexToId.isEmpty()) {
        idMapping.reserve(objectIndexToId.count());

        for (QHash<int, int>::ConstIterator idIt = objectIndexToId.constBegin(), end = objectIndexToId.constEnd();
             idIt != end; ++idIt) {

            const int objectIndex = idIt.key();
            QmlIR::JSCodeGen::IdMapping m;
            const QmlIR::Object *obj = qmlObjects.at(objectIndex);
            m.name = compiler->stringAt(obj->idIndex);
            m.idIndex = idIt.value();
            m.type = propertyCaches.at(objectIndex);

            QQmlCompiledData::TypeReference *tref = resolvedTypes.value(obj->inheritedTypeNameIndex);
            if (tref && tref->isFullyDynamicType)
                m.type = 0;

            idMapping << m;
        }
    }
    v4CodeGen->beginContextScope(idMapping, propertyCaches.at(contextObject));

    if (!compileJavaScriptCodeInObjectsRecursively(contextObject, contextObject))
        return false;

    return true;
}

bool JSCodeGenerator::compileJavaScriptCodeInObjectsRecursively(int objectIndex, int scopeObjectIndex)
{
    if (isComponent(objectIndex))
        return true;

    QmlIR::Object *object = qmlObjects.at(objectIndex);
    if (object->functionsAndExpressions->count > 0) {
        QQmlPropertyCache *scopeObject = propertyCaches.at(scopeObjectIndex);
        v4CodeGen->beginObjectScope(scopeObject);

        QList<QmlIR::CompiledFunctionOrExpression> functionsToCompile;
        for (QmlIR::CompiledFunctionOrExpression *foe = object->functionsAndExpressions->first; foe; foe = foe->next) {
            const bool haveCustomParser = customParsers.contains(object->inheritedTypeNameIndex);
            if (haveCustomParser)
                foe->disableAcceleratedLookups = true;
            functionsToCompile << *foe;
        }
        const QVector<int> runtimeFunctionIndices = v4CodeGen->generateJSCodeForFunctionsAndBindings(functionsToCompile);
        QList<QQmlError> jsErrors = v4CodeGen->qmlErrors();
        if (!jsErrors.isEmpty()) {
            foreach (const QQmlError &e, jsErrors)
                compiler->recordError(e);
            return false;
        }

        QQmlJS::MemoryPool *pool = compiler->memoryPool();
        object->runtimeFunctionIndices = pool->New<QmlIR::FixedPoolArray<int> >();
        object->runtimeFunctionIndices->init(pool, runtimeFunctionIndices);
    }

    for (const QmlIR::Binding *binding = object->firstBinding(); binding; binding = binding->next) {
        if (binding->type < QV4::CompiledData::Binding::Type_Object)
            continue;

        int target = binding->value.objectIndex;
        int scope = binding->type == QV4::CompiledData::Binding::Type_Object ? target : scopeObjectIndex;

        if (!compileJavaScriptCodeInObjectsRecursively(binding->value.objectIndex, scope))
            return false;
    }

    return true;
}

