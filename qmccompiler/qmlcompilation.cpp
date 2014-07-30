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
      document(NULL)
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
    checkData(&sizeInBytes);
    return sizeInBytes;
}

bool QmlCompilation::checkData(int *sizeInBytes) const
{
    // header
    int s = 0;
    int size = sizeof (QmcUnitHeader);
    if (sizeInBytes)
        *sizeInBytes = -1;
    if (type != QMC_QML && type != QMC_JS)
        return false;
    if (qmlUnit->qmlUnitSize > QMC_UNIT_MAX_QML_UNIT_SIZE)
        return false;
    if (unit->data->unitSize > QMC_UNIT_MAX_COMPILATION_UNIT_SIZE)
        return false;
    if (qmlUnit->nImports > QMC_UNIT_MAX_IMPORTS)
        return false;
    uint stringCount = unit->data->stringTableSize;
    if (stringCount > QMC_UNIT_MAX_STRINGS)
        return false;
    if (namespaces.size() > QMC_UNIT_MAX_NAMESPACES)
        return false;
    if (exportTypeRefs.size() > QMC_UNIT_MAX_TYPE_REFERENCES)
        return false;
    QV4::JIT::CompilationUnit *compilationUnit = (QV4::JIT::CompilationUnit *)unit;
    if (compilationUnit->codeRefs.size() > QMC_UNIT_MAX_CODE_REFS)
        return false;
    if (linkData.size() != compilationUnit->codeRefs.size())
        return false;
    if (compilationUnit->constantValues.size() != linkData.size())
        return false;
    if (objectIndexToIdRoot.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT)
        return false;
    if (objectIndexToIdComponent.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT)
        return false;
    if (aliases.size() > QMC_UNIT_MAX_ALIASES)
        return false;
    if (customParsers.size() > QMC_UNIT_MAX_CUSTOM_PARSERS)
        return false;
    if (customParserBindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS)
        return false;
    if (deferredBindings.size() > QMC_UNIT_MAX_DEFERRED_BINDINGS)
        return false;

    if (name.length() > QMC_UNIT_NAME_MAX_LEN)
        return false;
    if (url.toString().length() > QMC_UNIT_URL_MAX_LEN)
        return false;

    size += name.length() + 4;
    size += urlString.length() + 4;

    size += qmlUnit->nImports * sizeof(QV4::CompiledData::Import);

    for (uint i = 0; i < stringCount; i++) {
        s = compilationUnit->data->stringAt(i).length();
        if (s > QMC_UNIT_STRING_MAX_LEN)
            return false;
        size += s + 4;
    }

    foreach (const QString &ns, namespaces) {
        s = ns.length();
        if (s > QMC_UNIT_STRING_MAX_LEN)
            return false;
        size += s + 4;
    }

    size += exportTypeRefs.size() * sizeof (QmcUnitTypeReference);

    foreach (const JSC::MacroAssemblerCodeRef &codeRef, compilationUnit->codeRefs) {
        s = codeRef.size();
        if (s > QMC_UNIT_MAX_CODE_REF_SIZE)
            return false;
        size += s + 4;
    }

    foreach (const QVector<QmcUnitCodeRefLinkCall>& linkedCalls, linkData) {
        if (linkedCalls.size() > QMC_UNIT_MAX_CODE_REF_LINK_CALLS)
            return false;
        size += linkedCalls.size() * sizeof (QmcUnitCodeRefLinkCall) + sizeof(quint32);
    }

    foreach (const QVector<QV4::Primitive > & vec, compilationUnit->constantValues) {
        if (vec.size() > QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE)
            return false;
        size += vec.size() * sizeof (QV4::Primitive) + 4;
    }

    size += objectIndexToIdRoot.size() * sizeof (QmcUnitObjectIndexToId);

    size += objectIndexToIdComponent.size() * sizeof (quint32);
    foreach (const QmcUnitObjectIndexToIdComponent &componentMap, objectIndexToIdComponent) {
        if (componentMap.mappings.size() > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT_MAPPINGS)
            return false;
        size += componentMap.mappings.size() * sizeof (QmcUnitObjectIndexToId);
    }

    size += aliases.size() * sizeof (QmcUnitAlias);

    foreach (const QmcUnitCustomParser& customParser, customParsers) {
        size += sizeof (quint32);
        if (customParser.compilationArtifact.size() > QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH)
            return false;
        size += customParser.compilationArtifact.size();
        if (customParser.bindings.size() > QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH)
            return false;
        size += QMC_UNIT_BIT_ARRAY_LENGTH(customParser.bindings.size());
    }

    size += customParserBindings.size() * sizeof (quint32);

    foreach (const QmcUnitDeferredBinding &binding, deferredBindings) {
        size += sizeof (quint32);
        if (binding.bindings.size() > QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH)
            return false;
        size += QMC_UNIT_BIT_ARRAY_LENGTH(binding.bindings.size());
    }

    if (sizeInBytes)
        *sizeInBytes = size;

    return true;
}
