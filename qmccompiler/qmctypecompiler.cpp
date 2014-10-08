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

#include <QString>

#include <private/qqmltypenamecache_p.h>
#include <private/qqmlcompiler_p.h>

#include "qmctypecompiler.h"
#include "qmlcompilation.h"
#include "propertycachecreator.h"
#include "defaultpropertymerger.h"
#include "signalhandlerfunctionconverter.h"
#include "enumtyperesolver.h"
#include "customparserscriptindexer.h"
#include "aliasannotator.h"
#include "jscodegenerator.h"
#include "propertyvalidator.h"
#include "jsbindingexpressionsimplifier.h"
#include "componentandaliasresolver.h"
#include "scriptstringscanner.h"
#include "qmcinstructionselection.h"

#define tr(x) QString(x)

QmcTypeCompiler::QmcTypeCompiler(QmlCompilation *compilation)
    : compilation(compilation),
      compiledData(compilation->compiledData)
{
}

QmlCompilation* QmcTypeCompiler::data()
{
    return compilation;
}

void QmcTypeCompiler::recordError(const QQmlError &error)
{
    errors.append(error);
}

void QmcTypeCompiler::recordError(const QV4::CompiledData::Location& location, const QString& description)
{
    QQmlError error;
    error.setLine(location.line);
    error.setColumn(location.column);
    error.setUrl(compiledData->url);
    error.setDescription(description);
    recordError(error);
}

const QHash<int, QQmlCustomParser*> &QmcTypeCompiler::customParserCache() const
{
    return customParsers;
}

const QV4::Compiler::StringTableGenerator *QmcTypeCompiler::stringPool() const
{
    return &compilation->document->jsGenerator.stringTable;
}

QQmlJS::MemoryPool* QmcTypeCompiler::memoryPool()
{
    return compilation->document->jsParserEngine.pool();
}

QStringRef QmcTypeCompiler::newStringRef(const QString &string)
{
    return compilation->document->jsParserEngine.newStringRef(string);
}

int QmcTypeCompiler::registerString(const QString &str)
{
    return compilation->document->jsGenerator.registerString(str);
}

QHash<int, QQmlCompiledData::TypeReference*>* QmcTypeCompiler::resolvedTypes()
{
    return &compiledData->resolvedTypes;
}

QString QmcTypeCompiler::stringAt(int idx) const
{
    return compilation->document->stringAt(idx);
}

const QVector<QQmlPropertyCache *> &QmcTypeCompiler::propertyCaches() const
{
    return compiledData->propertyCaches;
}

QString QmcTypeCompiler::bindingAsString(const QmlIR::Object *object, int scriptIndex) const
{
    return object->bindingAsString(compilation->document, scriptIndex);
}

QList<QQmlError> QmcTypeCompiler::compilationErrors() const
{
    return errors;
}

QV4::IR::Module *QmcTypeCompiler::jsIRModule() const
{
    return &compilation->document->jsModule;
}

QHash<int, int> *QmcTypeCompiler::objectIndexToIdForRoot()
{
    return &compiledData->objectIndexToIdForRoot;
}

QHash<int, QHash<int, int> > *QmcTypeCompiler::objectIndexToIdPerComponent()
{
    return &compiledData->objectIndexToIdPerComponent;
}

QHash<int, QQmlCompiledData::CustomParserData> *QmcTypeCompiler::customParserData()
{
    return &compiledData->customParserData;
}

void QmcTypeCompiler::setCustomParserBindings(const QVector<int> &bindings)
{
    compiledData->customParserBindings = bindings;
}

void QmcTypeCompiler::setDeferredBindingsPerObject(const QHash<int, QBitArray> &deferredBindingsPerObject)
{
    compiledData->deferredBindingsPerObject = deferredBindingsPerObject;
}

QList<QmlIR::Object *> *QmcTypeCompiler::qmlObjects()
{
    return &compilation->document->objects;
}

int QmcTypeCompiler::rootObjectIndex() const
{
    return compilation->document->indexOfRootObject;
}

void QmcTypeCompiler::setVMEMetaObjects(const QVector<QByteArray> &metaObjects)
{
    Q_ASSERT(compiledData->metaObjects.isEmpty());
    compiledData->metaObjects = metaObjects;
}

void QmcTypeCompiler::setPropertyCaches(const QVector<QQmlPropertyCache *> &caches)
{
    compiledData->propertyCaches = caches;
    Q_ASSERT(caches.count() >= compilation->document->indexOfRootObject);
    if (compiledData->rootPropertyCache)
        compiledData->rootPropertyCache->release();
    compiledData->rootPropertyCache = caches.at(compilation->document->indexOfRootObject);
    compiledData->rootPropertyCache->addref();
}

void QmcTypeCompiler::scanScriptStrings()
{
    ScriptStringScanner sss(this);
    sss.scan();
}

QmlCompilation::TypeReference *QmcTypeCompiler::findTypeRef(int index)
{
    QHash<int, QmlCompilation::TypeReference> &refList = compilation->typeReferences;
    Q_ASSERT(refList.contains(index));
    return &refList[index];
}

void QmcTypeCompiler::setAliasIdToObjectIndex(const QHash<int, int> &idToObjectIndex)
{
    compilation->aliasIdToObjectIndex = idToObjectIndex;
}

void QmcTypeCompiler::appendAliasIdToObjectIndexPerComponent(int index, const QHash<int, int> &idToObjectIndex)
{
    compilation->aliasIdToObjectIndexPerComponent.insert(index, idToObjectIndex);
}

bool QmcTypeCompiler::createTypeMap()
{
    // qqmltypecompiler.cpp:81
    const QHash<int, QmlCompilation::TypeReference> &resolvedTypes = compilation->typeReferences;
    for (QHash<int, QmlCompilation::TypeReference>::ConstIterator resolvedType = resolvedTypes.constBegin(), end = resolvedTypes.constEnd();
         resolvedType != end; ++resolvedType) {
        QScopedPointer<QQmlCompiledData::TypeReference> ref(new QQmlCompiledData::TypeReference);
        QQmlType *qmlType = resolvedType->type;
        if (resolvedType.value().composite) {
            // qqmltypecompiler.cpp:87
            if (resolvedType->needsCreation && qmlType->isCompositeSingleton()) {
                QQmlError error;
                QString reason = tr("Composite Singleton Type %1 is not creatable.").arg(qmlType->qmlTypeName());
                error.setDescription(reason);
                error.setColumn(resolvedType->location.column);
                error.setLine(resolvedType->location.line);
                recordError(error);
                return false;
            }
            // qqmltypecompiler.cpp:96
            Q_ASSERT(resolvedType->component);
            Q_ASSERT(resolvedType->component->compiledData);
            ref->component = resolvedType->component->compiledData;
            ref->component->addref();
        } else if (qmlType) {
            ref->type = qmlType;
            Q_ASSERT(ref->type);

            if (resolvedType->needsCreation && !ref->type->isCreatable()) {
                QQmlError error;
                QString reason = ref->type->noCreationReason();
                if (reason.isEmpty())
                    reason = tr("Element is not creatable.");
                error.setDescription(reason);
                error.setColumn(resolvedType->location.column);
                error.setLine(resolvedType->location.line);
                error.setUrl(compilation->url);
                recordError(error);
                return false;
            }

            if (ref->type->containsRevisionedAttributes()) {
                QQmlError cacheError;
                ref->typePropertyCache = QQmlEnginePrivate::get(compilation->engine)->cache(
                                                        ref->type,
                                                        resolvedType->minorVersion,
                                                        cacheError);
                if (!ref->typePropertyCache) {
                    cacheError.setColumn(resolvedType->location.column);
                    cacheError.setLine(resolvedType->location.line);
                    recordError(cacheError);
                    return false;
                }
                ref->typePropertyCache->addref();
            }
        }
        ref->majorVersion = resolvedType->majorVersion;
        ref->minorVersion = resolvedType->minorVersion;
        ref->doDynamicTypeCheck();
        compiledData->resolvedTypes.insert(resolvedType.key(), ref.take());
    }
    return true;
}

bool QmcTypeCompiler::createPropertyCacheVmeMetaData()
{
    // initialize custom parser structure, will be populated in property validator
    for (QHash<int, QQmlCompiledData::TypeReference*>::ConstIterator it = compiledData->resolvedTypes.constBegin(), end = compiledData->resolvedTypes.constEnd();
         it != end; ++it) {
        QQmlCustomParser *customParser = (*it)->type ? (*it)->type->customParser() : 0;
        if (customParser)
            customParsers.insert(it.key(), customParser);
    }

    compiledData->metaObjects.reserve(compilation->document->objects.count());
    compiledData->propertyCaches.reserve(compilation->document->objects.count());

    // this will build meta objects and property caches
    PropertyCacheCreator propertyCacheBuilder(this);
    if (!propertyCacheBuilder.buildMetaObjects())
        return false;

    return true;
}

void QmcTypeCompiler::mergeDefaultProperties()
{
    DefaultPropertyMerger merger(this);
    merger.mergeDefaultProperties();
}

bool QmcTypeCompiler::convertSignalHandlersToFunctions()
{
    SignalHandlerFunctionConverter converter(this);
    return converter.convertSignalHandlerExpressionsToFunctionDeclarations();
}

bool QmcTypeCompiler::resolveEnums()
{
    EnumTypeResolver resolver(this);
    return resolver.resolveEnumBindings();
}

void QmcTypeCompiler::indexCustomParserScripts()
{
    CustomParserScriptIndexer cpi(this);
    cpi.annotateBindingsWithScriptStrings();
}

void QmcTypeCompiler::annotateAliases()
{
    AliasAnnotator annotator(this);
    annotator.annotateBindingsToAliases();
}

void QmcTypeCompiler::simplifyJavaScriptBindingExpressions()
{
    JSBindingExpressionSimplifier pass(this);
    pass.reduceTranslationBindings();
}

bool QmcTypeCompiler::validateProperties()
{
    PropertyValidator validator(this);
    return validator.validate();
}

bool QmcTypeCompiler::resolveComponentBoundariesAndAliases()
{
    ComponentAndAliasResolver resolver(this);
    return resolver.resolve();
}

bool QmcTypeCompiler::precompile()
{
    // QQmlTypeCompiler::compile
    // qqmltypecompiler.cpp:68
    compiledData->importCache = new QQmlTypeNameCache;

    // namespaces
    // qqmltypecompiler.cpp:72
    // qqmltypeloader.cpp:2356
    foreach (const QString &ns, compilation->namespaces) {
        compiledData->importCache->add(ns);
    }

    // TBD: qqmltypecompiler.cpp:72 namespaces, copy from QmlCompilation->namespaces

    // TBD: qqmltypecompiler.cpp:76 composite singletons

    compilation->importCache->populateCache(compiledData->importCache);

    // create typemap qqmltypecompiler.cpp:81
    if (!createTypeMap())
        return false;

    // qqmltypecompiler.cpp:134
    // Build property caches and VME meta object data
    if (!createPropertyCacheVmeMetaData()) {
        return false;
    }

    // default property merger
    mergeDefaultProperties();

    // convert signal handlers to functions
    if (!convertSignalHandlersToFunctions())
        return false;

    // resolve enums
    if (!resolveEnums())
        return false;

    // index custom parser scripts
    indexCustomParserScripts();

    // annotate aliases
    annotateAliases();

    // collect imported scripts
    // qqmltypecompiler.cpp:180
    // script data collection
    // 1. QQmlTypeData::continueLoadFromIR qqmltypeloader.cpp:2271 from QmlIR::Document -> QQmlTypeData::m_scripts
    // 2. QQmlTypeData::resolveTypes qqmltypeloader.cpp:2356 from QQmlImports -> QQmlTypeData::m_scripts
    // TBD: are actual script blobs needed ?
#if 0
    compiledData->scripts.reserve(compilation->scripts.count());
#endif
    for (int scriptIndex = 0; scriptIndex < compilation->scripts.count(); ++scriptIndex) {
        const QmlCompilation::ScriptReference &script = compilation->scripts.at(scriptIndex);

        QString qualifier = script.qualifier;
        QString enclosingNamespace;

        const int lastDotIndex = qualifier.lastIndexOf(QLatin1Char('.'));
        if (lastDotIndex != -1) {
            enclosingNamespace = qualifier.left(lastDotIndex);
            qualifier = qualifier.mid(lastDotIndex+1);
        }

        compiledData->importCache->add(qualifier, scriptIndex, enclosingNamespace);
#if 0
        QQmlScriptData *scriptData = script.script->scriptData();
        scriptData->addref();
        compiledData->scripts << scriptData;
#endif
    }

    // resolve component boundaries and aliases
    if (!resolveComponentBoundariesAndAliases())
        return false;

    // Compile JS binding expressions and signal handlers
    if (!compilation->document->javaScriptCompilationUnit) {
        // We can compile script strings ahead of time, but they must be compiled
        // without type optimizations as their scope is always entirely dynamic.
        scanScriptStrings();

        // TBD: check how it uses importCache
        // TBD: check how links with other scripts
        QmlIR::JSCodeGen v4CodeGenerator(compilation->urlString, compilation->document->code, &compilation->document->jsModule,
                                         &compilation->document->jsParserEngine, compilation->document->program,
                                         compiledData->importCache, &compilation->document->jsGenerator.stringTable);
        JSCodeGenerator jsCodeGen(this, &v4CodeGenerator);
        if (!jsCodeGen.generateCodeForComponents())
            return false;

        simplifyJavaScriptBindingExpressions();

        QQmlEnginePrivate *enginePrivate = QQmlEnginePrivate::get(compilation->engine);
        QV4::ExecutionEngine *v4 = enginePrivate->v4engine();
        // TBD: store unlinked binaries
#if 0
        QScopedPointer<QV4::EvalInstructionSelection> isel(
                    v4->iselFactory->create(enginePrivate, v4->executableAllocator,
                                            &compilation->document->jsModule, &compilation->document->jsGenerator));
#endif
        QScopedPointer<QmcInstructionSelection> isel(
                    new QmcInstructionSelection(enginePrivate, v4->executableAllocator,
                                            &compilation->document->jsModule, &compilation->document->jsGenerator));
        isel->setUseFastLookups(false);
        compilation->document->javaScriptCompilationUnit = isel->compile(/*generated unit data*/false);
        compilation->linkData = isel->linkData();
#if CPU(ARM_THUMB2)
        compilation->jumpsToLinkData = isel->linkRecordData();
        compilation->unlinkedCodeData = isel->unlinkedCodeData();
#endif
    }

    // Generate QML compiled type data structures

    QmlIR::QmlUnitGenerator qmlGenerator;
    QV4::CompiledData::QmlUnit *qmlUnit = qmlGenerator.generate(*compilation->document);

    Q_ASSERT(compilation->document->javaScriptCompilationUnit);
    Q_ASSERT((void*)qmlUnit == (void*)&qmlUnit->header);
    // The js unit owns the data and will free the qml unit.
    compilation->document->javaScriptCompilationUnit->data = &qmlUnit->header;

    compiledData->compilationUnit = compilation->document->javaScriptCompilationUnit;
    if (compiledData->compilationUnit)
        compiledData->compilationUnit->ref();
    compiledData->qmlUnit = qmlUnit; // ownership transferred to m_compiledData

    // add to type registry
    // qqmltypecompiler.cpp:248
    if (compiledData->isCompositeType())
        QQmlEnginePrivate::get(compilation->engine)->registerInternalCompositeType(compiledData);
    else {
        const QV4::CompiledData::Object *obj = qmlUnit->objectAt(qmlUnit->indexOfRootObject);
        QQmlCompiledData::TypeReference *typeRef = compiledData->resolvedTypes.value(obj->inheritedTypeNameIndex);
        Q_ASSERT(typeRef);
        if (typeRef->component) {
            compiledData->metaTypeId = typeRef->component->metaTypeId;
            compiledData->listMetaTypeId = typeRef->component->listMetaTypeId;
        } else {
            compiledData->metaTypeId = typeRef->type->typeId();
            compiledData->listMetaTypeId = typeRef->type->qListTypeId();
        }
    }

    // Sanity check property bindings and compile custom parsers
    if (!validateProperties())
        return false;

    return true;
}
