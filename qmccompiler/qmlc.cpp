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

#include  <QDir>
#include <private/qqmltypeloader_p.h>

#include "qmlc.h"
#include "qmcfile.h"
#include "qmcexporter.h"
#include "qmlcompilation.h"
#include "qmctypecompiler.h"

#include <private/qv4global_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmltypecompiler_p.h>
#include <private/qv8engine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qqmldebugserver_p.h>
#include <private/qqmlvmemetaobject_p.h>

int QmlC::MAX_RECURSION = 20;

QmlC::QmlC(QQmlEngine *engine, CompilerOptions *options, QObject *parent) :
    Compiler(engine, options, parent),
    implicitImportLoaded(false),
    recursion(0),
    components(new QHash<QString, QmlCompilation *>()),
    ownComponents(true)
{
}

QmlC::QmlC(QQmlEngine *engine, QHash<QString, QmlCompilation *> *components, CompilerOptions *options, QObject *parent) :
    Compiler(engine, options, parent),
    implicitImportLoaded(false),
    recursion(0),
    components(components),
    ownComponents(false)
{
}

QmlC::~QmlC()
{
    if (ownComponents) {
        foreach (QmlCompilation *compilation, *components) {
            delete compilation;
        }
        components->clear();
    }
}

bool QmlC::dataReceived()
{
    // qqmltypeloader.cpp:2207
    compilation()->document = new QmlIR::Document(false);
    QmlIR::IRBuilder compiler(QV8Engine::get(compilation()->engine)->illegalNames());

    if (!compiler.generateFromQml(compilation()->code, compilation()->name, compilation()->name, compilation()->document)) {
        QList<QQmlError> errors;
        foreach (const QQmlJS::DiagnosticMessage &msg, compiler.errors) {
            QQmlError e;
            e.setUrl(compilation()->url);
            e.setLine(msg.loc.startLine);
            e.setColumn(msg.loc.startColumn);
            e.setDescription(msg.message);
            errors << e;
        }
        appendErrors(errors);
        return false;
    }

    return continueLoadFromIR();
}

bool QmlC::continueLoadFromIR()
{
    compilation()->document->collectTypeReferences();
    QUrl baseUrl = compilation()->url;
    if (!compilation()->url.toLocalFile().startsWith(":/") &&
            !compilation()->urlString.startsWith("qrc:/") &&
            !compilation()->urlString.startsWith("file:/")) {
        QDir dd;
        QString newUrl = "file://" + dd.absolutePath() + "/" + compilation()->url.toLocalFile();
        compilation()->importCache->setBaseUrl(QUrl(newUrl), newUrl);
    }else{
        compilation()->importCache->setBaseUrl(baseUrl, compilation()->urlString);
    }

    // TBD: implicit import qqmltypeloader.cpp:2248

    QList<QQmlError> errors;

    foreach (const QV4::CompiledData::Import *import, compilation()->document->imports) {
        if (!addImport(import, &errors)) {
            // if errors are empty the error has already been reported
            if(errors.size()){
                QQmlError error(errors.takeFirst());
                error.setUrl(compilation()->importCache->baseUrl());
                error.setLine(import->location.line);
                error.setColumn(import->location.column);
                errors.prepend(error); // put it back on the list after filling out information.
                appendError(error);
            }
            return false;
        }
    }

    foreach (QmlIR::Pragma *pragma, compilation()->document->pragmas) {
        if (pragma->type == QmlIR::Pragma::PragmaSingleton) {
            compilation()->singleton = true;
        } else {
            QQmlError error;
            error.setDescription("Unknown pragma not supported");
            appendError(error);
            // TBD: qqmltypeloader.cpp:1413
            return false;
        }
    }
    return true;
}

bool QmlC::loadImplicitImport()
{
    // qqmltypeloader.cpp:2186
    implicitImportLoaded = true; // Even if we hit an error, count as loaded (we'd just keep hitting the error)

    if (!compilation()->url.toLocalFile().startsWith(":/") &&
            !compilation()->urlString.startsWith("qrc:/") &&
            !compilation()->urlString.startsWith("file:/")) {
        QDir dd;
        QString newUrl = "file://" + dd.absolutePath() + "/" + compilation()->url.toLocalFile();
        compilation()->importCache->setBaseUrl(QUrl(newUrl), newUrl);
    }else{
        compilation()->importCache->setBaseUrl(compilation()->url, compilation()->urlString);
    }


    QQmlImportDatabase *importDatabase = compilation()->importDatabase;
    // For local urls, add an implicit import "." as most overridden lookup.
    // This will also trigger the loading of the qmldir and the import of any native
    // types from available plugins.
    QList<QQmlError> implicitImportErrors;
    compilation()->importCache->addImplicitImport(importDatabase, &implicitImportErrors);

    if (!implicitImportErrors.isEmpty()) {
        appendErrors(implicitImportErrors);
        return false;
    }

    return true;

}

bool QmlC::resolveTypes()
{
    // script references
    // qqmltypeloader.cpp:2356
    // TBD
    foreach (const QQmlImports::ScriptReference &script, compilation()->importCache->resolvedScripts()) {
#if 0
        QmlCompilation *scriptCompilation = getComponent(script.location);
        if (!scriptCompilation) {
            QQmlError error;
            error.setDescription("Could not get dependency");
            error.setUrl(script.location);
            appendError(error);
            return false;
        }
#endif

        QmlCompilation::ScriptReference scriptRef;
        scriptRef.compilation = NULL; //scriptCompilation;

        scriptRef.qualifier = script.nameSpace;
        if (!script.qualifier.isEmpty())
        {
            scriptRef.qualifier.prepend(script.qualifier + QLatin1Char('.'));

            // Add a reference to the enclosing namespace
            if (!compilation()->namespaces.contains(script.qualifier))
                compilation()->namespaces.append(script.qualifier);
        }

        compilation()->scripts.append(scriptRef);
    }

    // TBD: qqmltypeloader.cpp:2377 resolved composite singletons
    // Lets handle resolved composite singleton types
    foreach (const QQmlImports::CompositeSingletonReference &csRef, compilation()->importCache->resolvedCompositeSingletons()) {
        QmlCompilation::TypeReference ref;
        QString typeName = csRef.typeName;
        QQmlImportNamespace *typeNamespace = 0;
        QList<QQmlError> errors;

        if (!csRef.prefix.isEmpty()) {
            typeName.prepend(csRef.prefix + QLatin1Char('.'));
            // Add a reference to the enclosing namespace
            compilation()->namespaces.append(csRef.prefix);
        }

        int majorVersion = -1;
        int minorVersion = -1;

        bool typeFound = compilation()->importCache->resolveType(typeName, &ref.type,
                                          &majorVersion, &minorVersion, &typeNamespace, &errors);

        if (!typeNamespace && !typeFound && !implicitImportLoaded) {
            if (loadImplicitImport()) {
                typeFound = compilation()->importCache->resolveType(typeName, &ref.type,
                                          &majorVersion, &minorVersion, &typeNamespace, &errors);
            }
            else {
                appendErrors(errors);
                return false; //loadImplicitImport() hit an error, and called setError already
            }
        }

        if (ref.type->isCompositeSingleton()) { 
            ref.name = typeName;
            ref.typeData = typeLoader()->getType(ref.type->sourceUrl());
            ref.prefix = csRef.prefix;
            compilation()->m_compositeSingletons << ref;
        }
    }

    // qqmltypeloader.cpp:2402 resolve type references
    const QV4::CompiledData::TypeReferenceMap &typeReferences = compilation()->document->typeReferences;
    for (QV4::CompiledData::TypeReferenceMap::ConstIterator unresolvedRef = typeReferences.constBegin(), end = typeReferences.constEnd();
         unresolvedRef != end; ++unresolvedRef) {

        int majorVersion = -1;
        int minorVersion = -1;
        QQmlImportNamespace *typeNamespace = 0;
        QList<QQmlError> errors;

        QmlCompilation::TypeReference ref;
        ref.name = stringAt(unresolvedRef.key());

        // check that exists
        bool typeFound = compilation()->importCache->resolveType(ref.name, &ref.type,
                &majorVersion, &minorVersion, &typeNamespace, &errors);
        if (!typeNamespace && !typeFound && !implicitImportLoaded) {
            // qqmltypeloader.cpp:2419 implicit import
            if (loadImplicitImport()) {
                // Try again to find the type
                errors.clear();
                typeFound = compilation()->importCache->resolveType(ref.name, &ref.type,
                    &majorVersion, &minorVersion, &typeNamespace, &errors);
            } else {
                appendErrors(errors);
                return false; //loadImplicitImport() hit an error, and called setError already
            }
        }

        if (!typeFound || typeNamespace) {
            // Known to not be a type:
            //  - known to be a namespace (Namespace {})
            //  - type with unknown namespace (UnknownNamespace.SomeType {})
            QQmlError error;
            if (typeNamespace) {
                error.setDescription(QString("Namespace %1 cannot be used as a type").arg(ref.name));
            } else {
                if (errors.size()) {
                    error = errors.takeFirst();
                } else {
                    // this should not be possible!
                    // Description should come from error provided by addImport() function.
                    error.setDescription(QQmlTypeLoader::tr("Unreported error adding script import to import database"));
                }
                error.setUrl(compilation()->importCache->baseUrl());
                error.setDescription(QString("%1 %2").arg(ref.name).arg(error.description()));
            }

            error.setLine(unresolvedRef->location.line);
            error.setColumn(unresolvedRef->location.column);

            errors.prepend(error);
            appendErrors(errors);
            return false;
        }

        ref.composite = false;

        // qqmltypeloader.cpp:2456 check composite type
        if (ref.type && ref.type->isComposite()) {
            ref.composite = true;
            ref.component = getComponent(ref.type->sourceUrl());
            if (!ref.component)
                return false;
        }

        ref.location.line = unresolvedRef->location.line;
        ref.location.column = unresolvedRef->location.column;
        ref.needsCreation = unresolvedRef->needsCreation;
        ref.majorVersion = majorVersion;
        ref.minorVersion = minorVersion;

        // add to map
        compilation()->typeReferences.insert(unresolvedRef.key(), ref);
    }

    return true;
}

bool QmlC::doCompile()
{
    // qqmltypeloader.cpp:2339 QQmlTypeData::compile
    QQmlCompiledData *compiledData = new QQmlCompiledData(compilation()->engine);
    compilation()->compiledData = compiledData;
    compiledData->url = compilation()->url;
    compiledData->name = compilation()->urlString;

    //qDebug() << "Compile" << compilation()->url;
    QmcTypeCompiler compiler(compilation(), options());
    if (!compiler.precompile()) {
        appendErrors(compiler.compilationErrors());
        return false;
    }
    return true;
}

bool QmlC::done()
{
    // TBD: qqmltypeloader.cpp:2103 check scripts for errors

    // qqmltypeloader.cpp:2119 check type references for errors
    // Check all composite singleton type dependencies for errors
    for (int ii = 0; ii < compilation()->m_compositeSingletons.count(); ++ii) {
        const QmlCompilation::TypeReference &type = compilation()->m_compositeSingletons.at(ii);
        if (!type.typeData) {

            return false;
        } else if (type.typeData && type.typeData->isError()) {

            return false;
        }
    }

    // If the type is CompositeSingleton but there was no pragma Singleton in the
    // QML file, lets report an error.
    QQmlType *type = QQmlMetaType::qmlType(compilation()->url, true);
    if (type && type->isCompositeSingleton() && !compilation()->singleton) {

        return false;
    }

    // qqmltypeloader.cpp:2169 compile
    return doCompile();
}

bool QmlC::createExportStructures()
{
    int i, ii;
    compilation()->qmlUnit = compilation()->compiledData->qmlUnit;
    compilation()->unit = compilation()->compiledData->compilationUnit;
    compilation()->unit->ref();

    for (i=0;i<compilation()->m_compositeSingletons.size();i++) {
        QmcUnitTypeReference typeRef;
        typeRef.syntheticComponent = 0;
        typeRef.composite = 1;
        for (ii=0;ii<compilation()->unit->data->stringTableSize;ii++) {
            if (compilation()->unit->data->stringAt(ii) == compilation()->m_compositeSingletons[i].name) {
                typeRef.index  = ii;
                compilation()->exportTypeRefs.append(typeRef);
                break;
            }
        }
    } 

    // resolved types list
    const QHash<int, QQmlCompiledData::TypeReference*> &typeRefHash = compilation()->compiledData->resolvedTypes;
    for (QHash<int, QQmlCompiledData::TypeReference*>::ConstIterator resolvedType = typeRefHash.constBegin(), end = typeRefHash.constEnd();
         resolvedType != end; ++resolvedType) {
        QmcUnitTypeReference typeRef;
        typeRef.index = resolvedType.key();
        QString name(compilation()->compiledData->compilationUnit->data->stringAt(typeRef.index));
        if (!name.compare(QString("QQmlComponent")))
            typeRef.syntheticComponent = 1;
        else
            typeRef.syntheticComponent = 0;

        if (resolvedType.value()->component)
            typeRef.composite = 1;
        else
            typeRef.composite = 0;

        //qDebug() << "Type ref" << typeRef.index << name;
        compilation()->exportTypeRefs.append(typeRef);
    }

    // script refs
    foreach (const QmlCompilation::ScriptReference script, compilation()->scripts) {
        QmcUnitScriptReference scriptRef;
        scriptRef.qualifier = script.qualifier;
        compilation()->exportScriptRefs.append(scriptRef);
    }

    // root object index to id mapping
    const QHash<int, int> &objectIdList = compilation()->compiledData->objectIndexToIdForRoot;
    for (QHash<int, int>::ConstIterator objectRef = objectIdList.constBegin(), end = objectIdList.constEnd();
         objectRef != end; objectRef++) {
        QmcUnitObjectIndexToId mapping;
        //qDebug() << "Object mapping" << objectRef.key() << " -> " << objectRef.value();
        mapping.index = objectRef.key();
        mapping.id = objectRef.value();
        compilation()->objectIndexToIdRoot.append(mapping);
    }

    // component root list + per-component object index to id mapping
    const QHash<int, QHash<int, int> > &objectIdListComponent = compilation()->compiledData->objectIndexToIdPerComponent;
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
        compilation()->objectIndexToIdComponent.append(mapping);
    }

    // collect aliases
    for (uint i = 0; i < compilation()->compiledData->qmlUnit->nObjects; i++) {
        const QV4::CompiledData::Object *obj = compilation()->compiledData->qmlUnit->objectAt(i);
        int effectiveAliasIndex = 0;
        for (uint j = 0; j < obj->nProperties; j++) {
            const QV4::CompiledData::Property *p = &obj->propertyTable()[j];
            if (p->type == QV4::CompiledData::Property::Alias) {
                QmcUnitAlias alias;
                alias.objectIndex = i;
                alias.propertyIndex = j;

                // check if it's a component/delegate to lookup
                if(compilation()->aliasIdToObjectIndexPerComponent.contains(alias.objectIndex)){
                    alias.targetObjectIndex = compilation()->aliasIdToObjectIndexPerComponent[alias.objectIndex].value(p->aliasIdValueIndex, -1);
                }else{
                    alias.targetObjectIndex = compilation()->aliasIdToObjectIndex.value(p->aliasIdValueIndex, -1);
                }

                if (alias.targetObjectIndex == -1) {
                    QHash<int, QHash<int, int> >::const_iterator iteratorAliases = compilation()->aliasIdToObjectIndexPerComponent.constBegin();
                    while (iteratorAliases != compilation()->aliasIdToObjectIndexPerComponent.constEnd()) {

                        alias.targetObjectIndex = iteratorAliases.value().value(p->aliasIdValueIndex, -1);
                        if (alias.targetObjectIndex != -1) {
                            break;
                        }    
                        ++iteratorAliases;
                    }
                }


                // qqmltypecompiler.cpp:1687
                typedef QQmlVMEMetaData VMD;
                QByteArray &dynamicData = compilation()->compiledData->metaObjects[i];
                Q_ASSERT(!dynamicData.isEmpty());
                VMD *vmd = (QQmlVMEMetaData *)dynamicData.data();
                QQmlVMEMetaData::AliasData *aliasData = &vmd->aliasData()[effectiveAliasIndex++];
                alias.contextIndex = aliasData->contextIdx;
                alias.targetPropertyIndex = aliasData->propertyIdx;
                alias.propertyType = aliasData->propType;
                alias.flags = aliasData->flags;
                alias.notifySignal = aliasData->notifySignal;
                compilation()->aliases.append(alias);
            }
        }
    }

    const QHash<int, QQmlCompiledData::CustomParserData> &customParsers = compilation()->compiledData->customParserData;
    for (QHash<int, QQmlCompiledData::CustomParserData>::ConstIterator customParserRef = customParsers.constBegin(), end = customParsers.constEnd();
         customParserRef != end; customParserRef++) {
        QmcUnitCustomParser customParser;
        customParser.objectIndex = customParserRef.key();
        customParser.compilationArtifact = customParserRef.value().compilationArtifact;
        customParser.bindings = customParserRef.value().bindings;
        compilation()->customParsers.append(customParser);
    }

    compilation()->customParserBindings = compilation()->compiledData->customParserBindings;

    const QHash<int, QBitArray> &deferredBindings = compilation()->compiledData->deferredBindingsPerObject;
    for (QHash<int, QBitArray>::ConstIterator ref = deferredBindings.constBegin(), end = deferredBindings.constEnd();
         ref != end; ref++) {
        QmcUnitDeferredBinding binding;
        binding.objectIndex = ref.key();
        binding.bindings = ref.value();
    }
    return true;
}

bool QmlC::compileData()
{
    compilation()->type = QMC_QML;

    // qqmltypeloader.cpp:2207 QQmlTypeData::dataReceived
    // -> qqmltypeloader.cpp: QQmlTypeData::continueLoadFromIR
    if (!dataReceived())
        return false;

    // TBD: check if there are unresolved imports

    // qqmltypeloader.cpp:2293 QQmlTypeData::allDependenciesDone
    // -> qqmltypeloader.cpp:2353 QQmlTypeData::resolveTypes
    if (!resolveTypes())
        return false;

    // qqmltypeloader.cpp:606 tryDone
    // -> qqmltypeloader.cpp:2100 QQmlTypeData::done
    if (!done()) {
        return false;
    }

    return true;
}

QmlCompilation* QmlC::getComponent(const QUrl& url)
{
    //qDebug() << "Load dependency" << url.toString();
    QString str = url.toString();
    if (components->contains(str))
        return (*components)[str];
    else {
        if (recursion > MAX_RECURSION) {
            QQmlError error;
            error.setUrl(url);
            error.setDescription("Max recursion reached");
            appendError(error);
            return NULL;
        }

        // compile
        QmlC compiler(engine(), components, options());
        compiler.recursion = this->recursion + 1;
        if (!compiler.compile(url.toString())) {
            appendErrors(compiler.errors());
            return NULL;
        }

        QmlCompilation* compilation = compiler.takeCompilation();
        components->insert(str, compilation);
        return compilation;
    }
}
