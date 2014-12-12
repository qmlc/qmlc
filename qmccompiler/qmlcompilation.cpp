/*!
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
 * In addition, as a special exception, copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

#include "qmlcompilation.h"
#include "qmcfile.h"

#include <private/qv4compiler_p.h>
#include <private/qqmlirbuilder_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qv4assembler_p.h>
#include <private/qqmldebugserver_p.h>
#include <private/qv4isel_masm_p.h>

QmlCompilation::QmlCompilation(const QString &urlString, const QUrl &url, QQmlEngine *engine)
    : urlString(urlString),
      url(url),
      compiledData(NULL),
      unit(NULL),
      engine(engine),
      document(NULL),
      singleton(false)
{
    if (QQmlDebugService::isDebuggingEnabled())
        // disable debugging
        QQmlEnginePrivate::get(engine)->v4engine()->iselFactory.reset(new QV4::JIT::ISelFactory);
}

QmlCompilation::~QmlCompilation()
{
    if (compiledData)
        compiledData->release();
    if (unit)
        unit->deref();
    if (document)
        delete document;
    if (importCache)
        delete importCache;
    if (importDatabase)
        delete importDatabase;
}

int QmlCompilation::calculateSize() const
{
    int sizeInBytes = -1;
    QQmlError error;
    checkData(error, &sizeInBytes);
    return sizeInBytes;
}

bool QmlCompilation::checkData(QQmlError &error, int *sizeInBytes) const
{
    // header
    int s = 0;
    int size = sizeof (QmcUnitHeader);
    if (sizeInBytes)
        *sizeInBytes = -1;
    if (type != QMC_QML && type != QMC_JS)
        return false;
    if (qmlUnit->qmlUnitSize > QMC_UNIT_MAX_QML_UNIT_SIZE){
        QString err = "qmlUnit->qmlUnitSize > QMC_UNIT_MAX_QML_UNIT_SIZE %1";
        err.arg(qmlUnit->qmlUnitSize);
        error.setDescription(err);
        return false;
    }
    if (unit->data->unitSize > QMC_UNIT_MAX_COMPILATION_UNIT_SIZE){
        QString err = "unit->data->unitSize > QMC_UNIT_MAX_COMPILATION_UNIT_SIZE %1";
        err.arg(unit->data->unitSize);
        error.setDescription(err);
        return false;
    }
    if (qmlUnit->nImports > QMC_UNIT_MAX_IMPORTS){
        QString err = "qmlUnit->nImports > QMC_UNIT_MAX_IMPORTS %1";
        err.arg(qmlUnit->nImports);
        error.setDescription(err);
        return false;
    }
    uint stringCount = unit->data->stringTableSize;
    if (stringCount > QMC_UNIT_MAX_STRINGS){
        QString err = "stringCount > QMC_UNIT_MAX_STRINGS %1";
        err.arg(stringCount);
        error.setDescription(err);
        return false;
    }
    if (namespaces.size() > QMC_UNIT_MAX_NAMESPACES){
        QString err = "namespaces.size() > QMC_UNIT_MAX_NAMESPACES %1";
        err.arg(namespaces.size());
        error.setDescription(err);
        return false;
    }
    if (exportTypeRefs.size() > QMC_UNIT_MAX_TYPE_REFERENCES){
        QString err = "exportTypeRefs.size() > QMC_UNIT_MAX_TYPE_REFERENCES %1";
        err.arg(exportTypeRefs.size());
        error.setDescription(err);
        return false;
    }
    QV4::JIT::CompilationUnit *compilationUnit = (QV4::JIT::CompilationUnit *)unit;
    if (compilationUnit->codeRefs.size() > QMC_UNIT_MAX_CODE_REFS){
        QString err = "compilationUnit->codeRefs.size() > QMC_UNIT_MAX_CODE_REFS %1";
        err.arg(compilationUnit->codeRefs.size());
        error.setDescription(err);
        return false;
    }
    if (linkData.size() != compilationUnit->codeRefs.size()){
        QString err = "linkData.size() != compilationUnit->codeRefs.size() %1 %2";
        err.arg(linkData.size()).arg(compilationUnit->codeRefs.size());
        error.setDescription(err);
        return false;
    }
    if (compilationUnit->constantValues.size() != linkData.size()){
        QString err = "compilationUnit->constantValues.size() != linkData.size() %1 %2";
        err.arg(compilationUnit->constantValues.size()).arg(linkData.size());
        error.setDescription(err);
        return false;
    }
    if (objectIndexToIdRoot.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT){
        QString err = "objectIndexToIdRoot.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT %1 %2";
        err.arg(objectIndexToIdRoot.size()).arg(QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT);
        error.setDescription(err);
        return false;
    }
    if (objectIndexToIdComponent.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT){
        QString err = "objectIndexToIdComponent.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT %1 %2";
        err.arg(objectIndexToIdComponent.size()).arg(QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT);
        error.setDescription(err);
        return false;
    }
    if (aliases.size() > QMC_UNIT_MAX_ALIASES){
        QString err = "aliases.size() > QMC_UNIT_MAX_ALIASES %1 %2";
        err.arg(aliases.size()).arg(QMC_UNIT_MAX_ALIASES);
        error.setDescription(err);
        return false;
    }
    if (customParsers.size() > QMC_UNIT_MAX_CUSTOM_PARSERS){
        QString err = "customParsers.size() > QMC_UNIT_MAX_CUSTOM_PARSERS %1 %2";
        err.arg(customParsers.size()).arg(QMC_UNIT_MAX_CUSTOM_PARSERS);
        error.setDescription(err);
        return false;
    }
    if (customParserBindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS){
        QString err = "customParserBindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS %1 %2";
        err.arg(customParserBindings.size()).arg(QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS);
        error.setDescription(err);
        return false;
    }
    if (deferredBindings.size() > QMC_UNIT_MAX_DEFERRED_BINDINGS){
        QString err = "deferredBindings.size() > QMC_UNIT_MAX_DEFERRED_BINDINGS %1 %2";
        err.arg(deferredBindings.size()).arg(QMC_UNIT_MAX_DEFERRED_BINDINGS);
        error.setDescription(err);
        return false;
    }

    if (name.length() > QMC_UNIT_NAME_MAX_LEN){
        QString err = "name.length() > QMC_UNIT_NAME_MAX_LEN %1 %2";
        err.arg(name.length()).arg(QMC_UNIT_NAME_MAX_LEN);
        error.setDescription(err);
        return false;
    }
    if (url.toString().length() > QMC_UNIT_URL_MAX_LEN){
        QString err = "url.toString().length() > QMC_UNIT_URL_MAX_LEN %1 %2";
        err.arg(url.toString().length()).arg(QMC_UNIT_URL_MAX_LEN);
        error.setDescription(err);
        return false;
    }

    size += name.length() + 4;
    size += urlString.length() + 4;

    size += qmlUnit->nImports * sizeof(QV4::CompiledData::Import);

    for (uint i = 0; i < stringCount; i++) {
        s = compilationUnit->data->stringAt(i).length();
        if (s > QMC_UNIT_STRING_MAX_LEN){
            QString err = "s > QMC_UNIT_STRING_MAX_LEN %1 %2";
            err = err.arg(s).arg(QMC_UNIT_STRING_MAX_LEN);
            error.setDescription(err);
            return false;
        }
        size += s + 4;
    }

    foreach (const QString &ns, namespaces) {
        s = ns.length();
        if (s > QMC_UNIT_STRING_MAX_LEN){
            QString err = "Namespaces s > QMC_UNIT_STRING_MAX_LEN %1 %2";
            err = err.arg(s).arg(QMC_UNIT_STRING_MAX_LEN);
            error.setDescription(err);
            return false;
        }
        size += s + 4;
    }

    size += exportTypeRefs.size() * sizeof (QmcUnitTypeReference);

    foreach (const JSC::MacroAssemblerCodeRef &codeRef, compilationUnit->codeRefs) {
        s = codeRef.size();
        if (s > QMC_UNIT_MAX_CODE_REF_SIZE){
            QString err = "s > QMC_UNIT_MAX_CODE_REF_SIZE %1 %2";
            err = err.arg(s).arg(QMC_UNIT_MAX_CODE_REF_SIZE);
            error.setDescription(err);
            return false;
        }
        size += s + 4;
    }

    foreach (const QVector<QmcUnitCodeRefLinkCall>& linkedCalls, linkData) {
        if (linkedCalls.size() > QMC_UNIT_MAX_CODE_REF_LINK_CALLS){
            QString err = "linkedCalls.size() > QMC_UNIT_MAX_CODE_REF_LINK_CALLS %1 %2";
            err.arg(linkedCalls.size()).arg(QMC_UNIT_MAX_CODE_REF_LINK_CALLS);
            error.setDescription(err);
            return false;
        }
        size += linkedCalls.size() * sizeof (QmcUnitCodeRefLinkCall) + sizeof(quint32);
    }

    foreach (const QVector<QV4::Primitive > & vec, compilationUnit->constantValues) {
        if (vec.size() > QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE){
            QString err = "vec.size() > QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE %1 %2";
            err.arg(vec.size()).arg(QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE);
            error.setDescription(err);
            return false;
        }
        size += vec.size() * sizeof (QV4::Primitive) + 4;
    }

    size += objectIndexToIdRoot.size() * sizeof (QmcUnitObjectIndexToId);

    size += objectIndexToIdComponent.size() * sizeof (quint32);
    foreach (const QmcUnitObjectIndexToIdComponent &componentMap, objectIndexToIdComponent) {
        if (componentMap.mappings.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT_MAPPINGS){
            QString err = "componentMap.mappings.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT_MAPPINGS %1 %2";
            err.arg(componentMap.mappings.size()).arg(QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT_MAPPINGS);
            error.setDescription(err);
            return false;
        }
        size += componentMap.mappings.size() * sizeof (QmcUnitObjectIndexToId);
    }

    size += aliases.size() * sizeof (QmcUnitAlias);

    foreach (const QmcUnitCustomParser& customParser, customParsers) {
        size += sizeof (quint32);
        if (customParser.compilationArtifact.size() > QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH){
            QString err = "customParser.compilationArtifact.size() > QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH %1 %2";
            err.arg(customParser.compilationArtifact.size()).arg(QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH);
            error.setDescription(err);
            return false;
        }
        size += customParser.compilationArtifact.size();
        if (customParser.bindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH){
            QString err = "customParser.bindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH %1 %2";
            err.arg(customParser.bindings.size()).arg(QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH);
            error.setDescription(err);
            return false;
        }
        size += QMC_UNIT_BIT_ARRAY_LENGTH(customParser.bindings.size());
    }

    size += customParserBindings.size() * sizeof (quint32);

    foreach (const QmcUnitDeferredBinding &binding, deferredBindings) {
        size += sizeof (quint32);
        if (binding.bindings.size() > QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH){
            QString err = "binding.bindings.size() > QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH %1 %2";
            err.arg(binding.bindings.size()).arg(QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH);
            error.setDescription(err);
            return false;
        }
        size += QMC_UNIT_BIT_ARRAY_LENGTH(binding.bindings.size());
    }

    if (sizeInBytes)
        *sizeInBytes = size;

    return true;
}
