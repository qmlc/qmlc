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

#include <QQmlEngine>
#include <QDir>

#include <private/qv4engine_p.h>
#include <private/qqmltypeloader_p.h>
#include <private/qqmltypecompiler_p.h>
#include <private/qqmlimport_p.h>
#include <private/qqmlvmemetaobject_p.h>

#include "qmctypeunit.h"
#include "qmcunit.h"
#include "qmcunitpropertycachecreator.h"
#include "qmcloader.h"
#include "qmcscriptunit.h"
#include "qmctypeunitcomponentandaliasresolver.h"

QmcTypeUnit::QmcTypeUnit(QmcUnit *qmcUnit, QQmlTypeLoader *typeLoader)
    : Blob(qmcUnit->loadedUrl, QQmlDataBlob::QmlFile, typeLoader),
      unit(qmcUnit),
      compiledData(new QQmlCompiledData(qmcUnit->engine)),
      linked(false),
      vmeMetaObjects(compiledData->metaObjects),
      propertyCaches(compiledData->propertyCaches),
      doneLinking(false)
{
    compiledData->url = qmcUnit->loadedUrl;
}


QmcTypeUnit::~QmcTypeUnit()
{
    foreach (const QQmlTypeData::ScriptReference &ref, scripts) {
        ref.script->release();
    }
    scripts.clear();

    foreach (QmcUnit *unit, dependencies) {
        unit->blob->release();
    }

    compiledData->release();
    delete unit;
}

QmcUnit *QmcTypeUnit::qmcUnit()
{
    return unit;
}

void QmcTypeUnit::initializeFromCachedUnit(const QQmlPrivate::CachedQmlUnit *)
{
}

void QmcTypeUnit::dataReceived(const Data &)
{
}

void QmcTypeUnit::done()
{
}

QString QmcTypeUnit::stringAt(int idx) const
{
    return unit->stringAt(idx);
}

bool QmcTypeUnit::link()
{
    if (linked)
        return true;

    linked = true;

    // create QV4::CompiledData::CompilationUnit and QQmlCompiledData
    compiledData->compilationUnit = unit->compilationUnit;
    compiledData->compilationUnit->ref();
    compiledData->qmlUnit = unit->qmlUnit;
    compiledData->name = unit->name;

    setStatus(Complete);

    // create imports
    if (!addImports())
        return false;

    // resolve dependencies (TBD: recursively)
    if (!initDependencies())
        return false;

    if (!initQml())
        return false;

    return true;
}

bool QmcTypeUnit::addImports()
{

    QList<QString> fileImports;

    if(unit->loadedUrl.toString().startsWith("file:///")){
        // full, path this must be a plugin we are adding
        m_importCache.setBaseUrl(QUrl(unit->loadedUrl), unit->loadedUrl.toString());
    }else if (unit->url.toLocalFile().startsWith(":/") || unit->urlString.startsWith("qrc:/")){
        m_importCache.setBaseUrl(unit->url, unit->urlString);
    }else{
        QDir dd;
        QString newUrl = "file://" + dd.absolutePath() + "/" + unit->loadedUrl.toLocalFile();
        m_importCache.setBaseUrl(QUrl(newUrl), newUrl);
    }

    compiledData->customParserData = qmcUnit()->customParsers;
    compiledData->customParserBindings = qmcUnit()->customParserBindings;
    compiledData->deferredBindingsPerObject = qmcUnit()->deferredBindings;

    // qqmltypeloader.cpp:2271
    // ->addImport qqmltypeloader.cpp:1311
    for (uint i = 0; i < compiledData->qmlUnit->nImports; i++) {
        const QV4::CompiledData::Import *p = compiledData->qmlUnit->importAt(i);
        if (p->type == QV4::CompiledData::Import::ImportScript) {
            // load it if it does not exist yet
            QmcScriptUnit* scriptUnit = unit->loader->getScript(stringAt(p->uriIndex), unit->loadedUrl);
            if (!scriptUnit) {
                QQmlError error;
                error.setColumn(p->location.column);
                error.setLine(p->location.line);
                error.setUrl(finalUrl());
                error.setDescription("Could not find imported script");
                unit->errors.append(error);
                return false;
            }
            QQmlTypeData::ScriptReference ref;
            ref.location = p->location;
            ref.qualifier = stringAt(p->qualifierIndex);
            ref.script = scriptUnit;
            scripts.append(ref);
        } else if (p->type == QV4::CompiledData::Import::ImportLibrary) {

            QString qmldirFilePath;
            QString qmldirUrl;
            const QString &importUri = stringAt(p->uriIndex);
            const QString &importQualifier = stringAt(p->qualifierIndex);

            if (QQmlMetaType::isLockedModule(importUri, p->majorVersion)) {
                if (!addImport(p, &unit->errors))
                    return false;
            } else if (m_importCache.locateQmldir(typeLoader()->importDatabase(), importUri, p->majorVersion, p->minorVersion,
                        &qmldirFilePath, &qmldirUrl)) {

                if(QFile::exists(qmldirFilePath + "_loader")){
                    qmldirFilePath += "_loader";
                    qDebug() << "Using qmldir_loader file" << qmldirFilePath;
                }

                // This is a local library import
                if (!m_importCache.addLibraryImport(typeLoader()->importDatabase(), importUri, importQualifier, p->majorVersion,
                            p->minorVersion, qmldirFilePath, qmldirUrl, false, &unit->errors)){
                    return false;
                }

                if (!importQualifier.isEmpty()) {
                    // Does this library contain any qualified scripts?
                    QUrl libraryUrl(qmldirUrl);
                    const QQmlTypeLoader::QmldirContent *qmldir = typeLoader()->qmldirContent(qmldirFilePath, qmldirUrl);
                    foreach (const QQmlDirParser::Script &script, qmldir->scripts()) {
                        QUrl scriptUrl = libraryUrl.resolved(QUrl(script.fileName));
                        QQmlScriptBlob *blob = typeLoader()->getScript(scriptUrl);
                        addDependency(blob);

                        scriptImported(blob, p->location, script.nameSpace, importQualifier);
                    }
                }


            } else {
                if (!addImport(p, &unit->errors))
                    return false;
            }

        } else if (p->type == QV4::CompiledData::Import::ImportFile) {
            // load file import
            // qqmltypeloader.cpp:1384
            const QString &importUri = stringAt(p->uriIndex);
            const QString &importQualifier = stringAt(p->qualifierIndex);

            QUrl qmldirUrl;
            if (importQualifier.isEmpty()) {
                qmldirUrl = finalUrl().resolved(QUrl(importUri + QLatin1String("/qmldir")));
                if (!QQmlImports::isLocal(qmldirUrl)) {
                    // This is a remote file; the import is currently incomplete
                    QQmlError error;
                    error.setDescription("Remote dependencies not supported");
                    error.setUrl(qmldirUrl);
                    unit->errors.append(error);
                    return false;
                }
            }

            if (!m_importCache.addFileImport(typeLoader()->importDatabase(), importUri, importQualifier, p->majorVersion,
                                       p->minorVersion, false, &unit->errors))
                return false;

            fileImports.append(importUri);

        } else {
            QQmlError error;
            error.setDescription("Unknown type import");
            error.setColumn(p->location.column);
            error.setLine(p->location.line);
            error.setUrl(finalUrl());
            unit->errors.append(error);
            return false;
        }
    }

    // resolve types
    // type data creation
    // qqmlirbuilder.cpp:285 create QV4::CompiledData::TypeReference = location
    // qqmltypeloader.cpp:2402 QV4::CompiledData::TypeReference -> QQmlTypeData::TypeReference
    // qqmltypecompiler.cpp:86 QQmlTypeData::TypeReference -> QQmlCompiledData::TypeReference

    foreach (const QmcUnitTypeReference& typeRef, unit->typeReferences) {
        int majorVersion = -1;
        int minorVersion = -1;
        QQmlImportNamespace *typeNamespace = 0;
        if ((int)typeRef.index >= unit->strings.size())
            return false;
        const QString name = stringAt(typeRef.index);
        if (typeRef.syntheticComponent)
            continue;
        QQmlCompiledData::TypeReference *ref = new QQmlCompiledData::TypeReference;
        QQmlType *qmlType = NULL;
        if (!m_importCache.resolveType(name, &qmlType, &majorVersion, &minorVersion, &typeNamespace, &unit->errors)) {
            bool found = false;
            // try to load it as implicit
            QmcUnit *typeUnit = qmcUnit()->loader->getType(name, unit->loadedUrl);
            if (typeUnit) {
                ref->component = ((QmcTypeUnit *)typeUnit->blob)->refCompiledData(); // addref
                unit->errors.clear();
                dependencies.append(typeUnit);
                found = true;
            }
            if(!found){
                // local file imports
                foreach (const QString &path, fileImports){
                    QmcUnit *typeUnit = qmcUnit()->loader->getType(path + "/" + name, unit->loadedUrl);
                    if (typeUnit) {
                        ref->component = ((QmcTypeUnit *)typeUnit->blob)->refCompiledData(); // addref
                        unit->errors.clear();
                        dependencies.append(typeUnit);
                        found = true;
                        break;
                    }
                }
            }
            if(!found){
                QQmlError error;
                error.setDescription("Could not load implicit import");
                unit->errors.append(error);
                delete ref;
                return false;
            }
        }

        // component creation, see qqmltypecompiler.cpp:87
        // and qqmltypeloader.cpp:2456
        if (typeRef.composite && !ref->component) {
            // extract name of the source url
            Q_ASSERT(qmlType);
            QString sourceName;
            if (!sourceNameForUrl(qmlType->sourceUrl(), sourceName)) {
                delete ref;
                return false;
            }

            int lastDot = sourceName.lastIndexOf('.');
            if (lastDot != -1)
                sourceName = sourceName.left(lastDot);

            QmcUnit *typeUnit = qmcUnit()->loader->getType(sourceName, unit->loadedUrl);
            if (typeUnit) {
                ref->component = ((QmcTypeUnit *)typeUnit->blob)->refCompiledData(); // addref
                dependencies.append(typeUnit);
            } else {
                QQmlError error;
                error.setDescription("Could not load implicit import");
                unit->errors.append(error);
                delete ref;
                return false;
            }
        } else if (qmlType) {
            ref->type = qmlType;
            if (ref->type->containsRevisionedAttributes()) {
                // qqmltypecompiler.cpp:102
                QQmlError cacheError;
                ref->typePropertyCache = QQmlEnginePrivate::get(unit->engine)->cache(ref->type, minorVersion, cacheError);

                if (!ref->typePropertyCache) {
                    delete ref;
                    unit->errors.append(cacheError);
                    return false;
                }
                ref->typePropertyCache->addref();
            }
        }
        // TBD: qqmltypecompiler.cpp:86

        ref->majorVersion = majorVersion;
        ref->minorVersion = minorVersion;

        // dynamic type check moved to init phase

        compiledData->resolvedTypes.insert(typeRef.index, ref);
    }

    // from QQmlTypeCompiler::compile
    compiledData->importCache = new QQmlTypeNameCache;

    foreach (const QString &ns, unit->namespaces)
        compiledData->importCache->add(ns);
#if 0
    // Add any Composite Singletons that were used to the import cache
    foreach (const QQmlTypeData::TypeReference &singleton, compositeSingletons)
        compiledData->importCache->add(singleton.type->qmlTypeName(), singleton.type->sourceUrl(), singleton.prefix);
#endif

    // TBD: is import cache required at all ? Cannot be null though.
    m_importCache.populateCache(compiledData->importCache);

    // qqmltypecompiler.cpp:1413
    foreach (const QmcUnitTypeReference& typeRef, unit->typeReferences) {
        static QQmlType *componentType = QQmlMetaType::qmlType(&QQmlComponent::staticMetaObject);
        if (!typeRef.syntheticComponent)
            continue;
        QQmlCompiledData::TypeReference *ref = new QQmlCompiledData::TypeReference;
        ref->type = componentType;
        ref->majorVersion = componentType->majorVersion();
        ref->minorVersion = componentType->minorVersion();

        compiledData->resolvedTypes.insert(typeRef.index, ref);
    }

    // scripts resolved through file imports
    // actual script loading is done in QQmlObjectCreator::create (qqmlobjectcreator.cpp:215)
    foreach (const QQmlImports::ScriptReference &scriptRef, imports().resolvedScripts()) {
        // script reference
        QString locationName;
        if (!sourceNameForUrl(scriptRef.location, locationName))
            return false;

        QmcScriptUnit *script = unit->loader->getScript(locationName, unit->loadedUrl);
        if (!script) {
            QQmlError error;
            error.setDescription("Could not load script");
            error.setUrl(scriptRef.location);
            unit->errors.append(error);
            return false;
        }
        //compiledData->scripts.append(script->scriptData());
        QQmlTypeData::ScriptReference ref;
        ref.script = script;
        ref.qualifier = scriptRef.qualifier;
        scripts.append(ref);
    }

    for (int scriptIndex = 0; scriptIndex < unit->scriptReferences.count(); ++scriptIndex) {

        QString qualifier = unit->scriptReferences[scriptIndex];
        QString enclosingNamespace;

        const int lastDotIndex = qualifier.lastIndexOf(QLatin1Char('.'));
        if (lastDotIndex != -1) {
            enclosingNamespace = qualifier.left(lastDotIndex);
            qualifier = qualifier.mid(lastDotIndex+1);
        }

        compiledData->importCache->add(qualifier, scriptIndex, enclosingNamespace);
    }

    return true;
}

bool QmcTypeUnit::sourceNameForUrl(const QUrl &url, QString &name)
{
    name = url.toString();
    QString loadedBaseUrl = QmcLoader::getBaseUrl(unit->loadedUrl.toString());

    if (name.startsWith("file://")) {
        // keep name as is
    } else if (name.startsWith(loadedBaseUrl)) {
        if (name.size() > loadedBaseUrl.size())
            name = name.mid(loadedBaseUrl.size());
        else
            name = "";
    } else {
        // use just name
        qDebug() << "Reverting back to using just file name to resolve url, propably won't work";
        int lastSlash = name.lastIndexOf('/');
        if (lastSlash + 1 >= name.length()) {
            QQmlError error;
            error.setDescription("Illegal formatted url");
            error.setUrl(url);
            unit->errors.append(error);
            return false;
        } else if (lastSlash != -1)
            name = name.mid(lastSlash + 1);
    }
    return true;
}

QQmlCompiledData *QmcTypeUnit::refCompiledData()
{
    Q_ASSERT(compiledData);
    compiledData->addref();
    return compiledData;
}

bool QmcTypeUnit::initDependencies()
{
    foreach (const QQmlTypeData::ScriptReference &scriptRef, scripts) {
        QmcScriptUnit *script = (QmcScriptUnit *)scriptRef.script;
        if (!script->initialize())
            return false;
    }

    foreach (QmcUnit *unit, dependencies) {
        if (unit->type == QMC_QML) {
            QmcTypeUnit * blob = (QmcTypeUnit *)unit->blob;
            if (!blob->link())
                return false;
        }
    }

    // TBD: initialize dependencies of script & types (recursion)
    return true;
}

bool QmcTypeUnit::initQml()
{
    // type references init
    foreach (QQmlCompiledData::TypeReference *typeRef, compiledData->resolvedTypes) {
        typeRef->doDynamicTypeCheck();
    }

    compiledData->initialize(unit->engine);

    // create property caches
    // qqmltypecompiler.cpp:143-150
    QmcUnitPropertyCacheCreator cacheCreator(this);
    if (!cacheCreator.buildMetaObjects())
        return false;

    // scripts
    // qqmltypeloader.cpp:1315:
    // create QQmlScriptBlob + QQmlScriptData
    // add to QQmlTypeLoader::Blob->scripts
    // qqmltypecompiler.cpp:180:
    // add to import cache
    // QQmlTypeData::ScriptReference->scriptData -> compiledData->scripts

    foreach (const QQmlTypeData::ScriptReference &scriptRef, scripts) {
        // create QQmlScriptData and link it to QmcScriptUnit
        QQmlScriptData *scriptData = scriptRef.script->scriptData();
        scriptData->addref();
        compiledData->scripts.append(scriptData);
    }

    // add object mappings + aliases
    // alias creation call chain
    // QQmlTypeCompiler::compile
    // ->QQmlComponentAndAliasResolver::resolve->resolveAliases
    // -->QQmlPropertyCache::appendProperty
    // TBD: add aliases to property cache
    QmcTypeUnitComponentAndAliasResolver resolver(this);
    if (!resolver.resolve())
        return false;

    // TBD: alias creation makes component composite type
    if (compiledData->isCompositeType()) {
        // TBD: does this work ?
        QQmlEnginePrivate::get(unit->engine)->registerInternalCompositeType(compiledData);
        //engine->registerInternalCompositeType(compiledData);
    } else {
        const QV4::CompiledData::Object *obj = compiledData->qmlUnit->objectAt(compiledData->qmlUnit->indexOfRootObject);
        if (!obj)
            return false;
        QQmlCompiledData::TypeReference *typeRef = compiledData->resolvedTypes.value(obj->inheritedTypeNameIndex);
        if (!typeRef)
            return false;

        if (typeRef->component) {
            compiledData->metaTypeId = typeRef->component->metaTypeId;
            compiledData->listMetaTypeId = typeRef->component->listMetaTypeId;
        } else {
            compiledData->metaTypeId = typeRef->type->typeId();
            compiledData->listMetaTypeId = typeRef->type->qListTypeId();
        }
    }

    // extra initiliazation of QQmlCompiledData qqmltypecompiler.cpp:263
#if 0
    // TBD: function below seems to quite a lot
    // Sanity check property bindings
    QQmlPropertyValidator validator(this);
    if (!validator.validate())
        return false;
#endif
    // add custom parsers, custom bindings and deferred bindings
    // TBD:

    // Collect some data for instantiation later.
    int bindingCount = 0;
    int parserStatusCount = 0;
    int objectCount = 0;
    for (quint32 i = 0; i < compiledData->qmlUnit->nObjects; ++i) {
        const QV4::CompiledData::Object *obj = compiledData->qmlUnit->objectAt(i);
        bindingCount += obj->nBindings;
        if (QQmlCompiledData::TypeReference *typeRef = compiledData->resolvedTypes.value(obj->inheritedTypeNameIndex)) {
            if (QQmlType *qmlType = typeRef->type) {
                if (qmlType->parserStatusCast() != -1)
                    ++parserStatusCount;
            }
            if (typeRef->component) {
                bindingCount += typeRef->component->totalBindingsCount;
                parserStatusCount += typeRef->component->totalParserStatusCount;
                objectCount += typeRef->component->totalObjectCount;
            } else
                ++objectCount;
        }
    }

    compiledData->totalBindingsCount = bindingCount;
    compiledData->totalParserStatusCount = parserStatusCount;
    compiledData->totalObjectCount = objectCount;
    if (compiledData->propertyCaches.count() != static_cast<int>(compiledData->qmlUnit->nObjects))
        return false;

    return true;
}

QQmlComponent* QmcTypeUnit::createComponent()
{
    QQmlComponent* component = new QQmlComponent(unit->engine);
    QQmlComponentPrivate* cPriv = QQmlComponentPrivate::get(component);
    cPriv->cc = compiledData;
    cPriv->cc->addref();
    return component;
}
