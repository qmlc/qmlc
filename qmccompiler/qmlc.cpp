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

#include "qmlc.h"
#include "qmcfile.h"
#include "qmcexporter.h"
#include "qmlcompilation.h"

#include <private/qv4global_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmltypecompiler_p.h>
#include <private/qv8engine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qqmldebugserver_p.h>
#include <private/qqmlvmemetaobject_p.h>

QmlC::QmlC(QObject *parent) :
    Compiler(parent)
{
}

QmlC::~QmlC()
{
}

bool QmlC::compileData(QmlCompilation *compilation)
{
    compilation->type = QMC_QML;
    QQmlTypeData *typeData = QQmlEnginePrivate::get(compilation->engine)->typeLoader.getType(compilation->code.toUtf8(), compilation->url);
    if (!typeData)
        return false;

    if (typeData->isError()) {
        appendErrors(typeData->errors());
        typeData->release();
        return false;
    }

    if (!typeData->isComplete()) {
        typeData->release();
        return false;
    }

    compilation->compiledData = typeData->compiledData();
    compilation->compiledData->addref();
    compilation->typeData = typeData;
    compilation->name = compilation->compiledData->name;
    compilation->qmlUnit = compilation->compiledData->qmlUnit;
    compilation->unit = compilation->compiledData->compilationUnit;
    compilation->unit->ref();
    foreach (const QString &ns, typeData->namespaces()) {
        compilation->namespaces.append(ns);
    }

    const QHash<int, QQmlCompiledData::TypeReference*> &typeRefHash = compilation->compiledData->resolvedTypes;
    for (QHash<int, QQmlCompiledData::TypeReference*>::ConstIterator resolvedType = typeRefHash.constBegin(), end = typeRefHash.constEnd();
         resolvedType != end; ++resolvedType) {
        QmcUnitTypeReference typeRef;
        typeRef.index = resolvedType.key();
        QString name(compilation->compiledData->compilationUnit->data->stringAt(typeRef.index));
        if (!name.compare(QString("QQmlComponent")))
            typeRef.syntheticComponent = 1;
        else
            typeRef.syntheticComponent = 0;
        //qDebug() << "Type ref" << typeRef.index << name;
        compilation->typeRefs.append(typeRef);
    }


    const QHash<int, int> &objectIdList = compilation->compiledData->objectIndexToIdForRoot;
    for (QHash<int, int>::ConstIterator objectRef = objectIdList.constBegin(), end = objectIdList.constEnd();
         objectRef != end; objectRef++) {
        QmcUnitObjectIndexToId mapping;
        //qDebug() << "Object mapping" << objectRef.key() << " -> " << objectRef.value();
        mapping.index = objectRef.key();
        mapping.id = objectRef.value();
        compilation->objectIndexToIdRoot.append(mapping);
    }

    const QHash<int, QHash<int, int> > &objectIdListComponent = compilation->compiledData->objectIndexToIdPerComponent;
    for (QHash<int, QHash<int, int> >::ConstIterator componentRef = objectIdListComponent.constBegin(), end = objectIdListComponent.constEnd();
         componentRef != end; componentRef++) {
        const QHash<int, int>& componentRefTable = componentRef.value();
        QmcUnitObjectIndexToIdComponent mapping;
        int mappingCount = componentRefTable.size();
        mapping.componentIndex = componentRef.key();
        if (mappingCount > 0) {
            mapping.mappings.resize(mappingCount);
            int i = 0;
            for (QHash<int, int>::ConstIterator objectRef = componentRefTable.constBegin(), componentEnd = componentRefTable.constEnd();
                 objectRef != componentEnd; objectRef++) {
                //qDebug() << "Component" << componentRef.key() << "Object mapping" << objectRef.key() << " -> " << objectRef.value() << "i" << i;
                QmcUnitObjectIndexToId& objectMapping = mapping.mappings[i++];
                objectMapping.index = objectRef.key();
                objectMapping.id = objectRef.value();
            }
        }
        compilation->objectIndexToIdComponent.append(mapping);
    }

    // collect aliases
    for (uint i = 0; i < compilation->compiledData->qmlUnit->nObjects; i++) {
        const QV4::CompiledData::Object *obj = compilation->compiledData->qmlUnit->objectAt(i);
        int effectiveAliasIndex = 0;
        for (uint j = 0; j < obj->nProperties; j++) {
            const QV4::CompiledData::Property *p = &obj->propertyTable()[j];
            if (p->type == QV4::CompiledData::Property::Alias) {
                QmcUnitAlias alias;
                alias.objectIndex = i;
                alias.propertyIndex = j;

                // qqmltypecompiler.cpp:1687
                typedef QQmlVMEMetaData VMD;
                QByteArray &dynamicData = compilation->compiledData->metaObjects[i];
                Q_ASSERT(!dynamicData.isEmpty());
                VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
                QQmlVMEMetaData::AliasData *aliasData = &vmd->aliasData()[effectiveAliasIndex++];
                alias.contextIndex = aliasData->contextIdx;
                alias.targetPropertyIndex = aliasData->propertyIdx;
                alias.propertyType = aliasData->propType;
                alias.flags = aliasData->flags;
                alias.notifySignal = aliasData->notifySignal;
                compilation->aliases.append(alias);
            }
        }
    }

    const QHash<int, QQmlCompiledData::CustomParserData> &customParsers = compilation->compiledData->customParserData;
    for (QHash<int, QQmlCompiledData::CustomParserData>::ConstIterator customParserRef = customParsers.constBegin(), end = customParsers.constEnd();
         customParserRef != end; customParserRef++) {
        QmcUnitCustomParser customParser;
        customParser.objectIndex = customParserRef.key();
        customParser.compilationArtifact = customParserRef.value().compilationArtifact;
        customParser.bindings = customParserRef.value().bindings;
        compilation->customParsers.append(customParser);
    }

    compilation->customParserBindings = compilation->compiledData->customParserBindings;

    const QHash<int, QBitArray> &deferredBindings = compilation->compiledData->deferredBindingsPerObject;
    for (QHash<int, QBitArray>::ConstIterator ref = deferredBindings.constBegin(), end = deferredBindings.constEnd();
         ref != end; ref++) {
        QmcUnitDeferredBinding binding;
        binding.objectIndex = ref.key();
        binding.bindings = ref.value();
    }

    return true;
}
