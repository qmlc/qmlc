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

#include "qmcunit.h"

#include <sys/mman.h>
#include <sys/user.h>

#include <private/qv4assembler_p.h>
#include <private/qv4executableallocator_p.h>

#include "ExecutableAllocator.h"

#include "qmcunitpropertycachecreator.h"
#include "qmctypeunit.h"
#include "qmcscriptunit.h"

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE

QmcUnit::QmcUnit(QmcUnitHeader *header, const QUrl& url, const QString &urlString, QQmlEngine *engine, QmcLoader *loader, const QString &name) :
    engine(engine),
    header(header),
    qmlUnit(NULL),
    unit(NULL),
    compilationUnit(new QV4::JIT::CompilationUnit),
    url(url),
    urlString(urlString),
    type((QmcFileType)header->type),
    loader(loader),
    name(name)
{
    compilationUnit->ref();
    QQmlTypeLoader *typeLoader = &QQmlEnginePrivate::get(engine)->typeLoader;
    if (type == QMC_QML)
        blob = new QmcTypeUnit(this, typeLoader);
    else
        blob = new QmcScriptUnit(this, typeLoader);
}

QmcUnit::~QmcUnit()
{
    delete header;
    delete qmlUnit;
    compilationUnit->deref();
    for (int i = 0; i < allocations.size(); i++) {
        QV4::ExecutableAllocator::Allocation *a = allocations[i];
        a->deallocate(QQmlEnginePrivate::get(engine)->v4engine()->executableAllocator);
    }
    allocations.clear();
}

QmcUnit *QmcUnit::loadUnit(QDataStream &stream, QQmlEngine *engine, QmcLoader *loader)
{
    QmcUnitHeader *header = new QmcUnitHeader;

    bool ret = readData((char *)header, sizeof(QmcUnitHeader), stream);
    if (!ret || !checkHeader(header)) {
        delete header;
        return NULL;
    }

    QString name;
    QString urlString;
    if (!readString(name, stream) || !readString(urlString, stream)) {
        delete header;
        return NULL;
    }

    QUrl url;
    url.setUrl(urlString);

    QmcUnit *unit = new QmcUnit(header, url, urlString, engine, loader, name);

    if (unit->loadUnitData(stream))
        return unit;

    unit->blob->release();
    return NULL;
}

bool QmcUnit::loadUnitData(QDataStream &stream)
{
    char *qmlUnitPtr = new char[header->sizeQmlUnit];
    char *dataPtr = new char[header->sizeUnit];
    qmlUnit = reinterpret_cast<QV4::CompiledData::QmlUnit*>(qmlUnitPtr);
    unit = reinterpret_cast<QV4::CompiledData::Unit*>(dataPtr);
    compilationUnit->data = unit;
    if (!qmlUnit || !unit)
        return false;

    if (!readData(qmlUnitPtr, header->sizeQmlUnit, stream))
        return false;

    if (!readData(dataPtr, header->sizeUnit, stream))
        return false;

    // load imports
    for (int i = 0; i < (int)header->imports; i++) {
        QV4::CompiledData::Import import;
        if (!readData((char *)&import, sizeof(QV4::CompiledData::Import), stream))
            return false;
        if (import.uriIndex >= header->strings || import.qualifierIndex >= header->strings)
            return false;
        imports.append(import);
    }

    for (int i = 0; i < (int)header->strings; i++) {
        QString string;
        if (!readString(string, stream))
            return false;
        strings.append(string);
    }

    for (int i = 0; i < (int)header->namespaces; i++) {
        QString ns;
        if (!readString(ns, stream))
            return false;
        namespaces.append(ns);
    }

    for (int i = 0; i < (int)header->typeReferences; i++) {
        QmcUnitTypeReference typeRef;
        if (!readData((char *)&typeRef, sizeof (QmcUnitTypeReference), stream))
            return false;
        typeReferences.append(typeRef);
    }

    // coderefs
    for (int i = 0; i < (int)header->codeRefs; i++) {
        quint32 codeRefLen = 0;
        if (!readData((char *)&codeRefLen, sizeof(quint32), stream))
            return false;
        if (codeRefLen > QMC_UNIT_MAX_CODE_REF_SIZE)
            return false;
        if (codeRefLen == 0) {
            JSC::MacroAssemblerCodeRef codeRef;
            compilationUnit->codeRefs.append(codeRef);
            continue;
        }
        QV4::ExecutableAllocator::Allocation *executableMemory =
                QQmlEnginePrivate::get(engine)->v4engine()->executableAllocator->allocate(codeRefLen);
        if (!executableMemory)
            return false;
        allocations.append(executableMemory);
        char *code = (char *) executableMemory->start();
        //ASSERT(code);
        JSC::ExecutableAllocator::makeWritable(code, codeRefLen);

        if (!readData(code, codeRefLen, stream))
            return false;

        JSC::MacroAssemblerCodePtr codePtr = JSC::MacroAssemblerCodePtr::createFromExecutableAddress(code);
        JSC::MacroAssemblerCodeRef codeRef = JSC::MacroAssemblerCodeRef::createSelfManagedCodeRef(codePtr);

        compilationUnit->codeRefs.append(codeRef);
    }

    // constants
    for (int i = 0; i < (int)header->constantVectors; i++) {
        quint32 constantVectorLen = 0;
        if (!readData((char *)&constantVectorLen, sizeof(quint32), stream))
            return false;
        if (constantVectorLen > QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE)
            return false;
        QVector<QV4::Primitive > constantVector(constantVectorLen);
        compilationUnit->constantValues.append(constantVector);
        if (constantVectorLen == 0)
            continue;
        if (!readData((char *)constantVector.data(), constantVectorLen, stream))
            return false;
    }

    // object index -> id
    for (uint i = 0; i < header->objectIndexToIdRoot; i++) {
        QmcUnitObjectIndexToId mapping;
        if (!readData((char *)&mapping, sizeof (QmcUnitObjectIndexToId), stream))
            return false;
        objectIndexToIdRoot.append(mapping);
    }

    // component index + object index -> id
    for (uint i = 0; i < header->objectIndexToIdComponent; i++) {
        QmcUnitObjectIndexToIdComponent mapping;
        if (!readData((char *)&mapping, sizeof (QmcUnitObjectIndexToIdComponent), stream))
            return false;
        objectIndexToIdComponent.append(mapping);
    }

    for (uint i = 0; i < header->aliases; i++) {
        QmcUnitAlias alias;
        if (!readData((char *)&alias, sizeof (QmcUnitAlias), stream))
            return false;
        aliases.append(alias);
    }

    for (uint i = 0; i < header->customParsers; i++) {
        quint32 objectIndex = 0;
        if (!readData((char *)&objectIndex, sizeof (quint32), stream))
            return false;
        quint32 len = 0;
        if (!readData((char *)&len, sizeof (quint32), stream))
            return false;
        if (len > QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH)
            return false;
        QQmlCompiledData::CustomParserData customParserData;
        QByteArray artifact;
        artifact.resize(len);
        if (!readData(artifact.data(), (int)len, stream))
            return false;
        if (!readData((char *)&len, sizeof (quint32), stream))
            return false;
        if (len > QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH)
            return false;
        QBitArray bindings;
        bindings.resize(len);
        if (!readBitArray(bindings, stream))
            return false;
        customParserData.compilationArtifact = artifact;
        customParserData.bindings = bindings;
        customParsers.insert(objectIndex, customParserData);
    }

    customParserBindings.resize(header->customParserBindings);
    for (uint i = 0; i < header->customParserBindings; i++) {
        quint32 d = 0;
        if (!readData((char *)&d, sizeof (quint32), stream))
            return false;
        customParserBindings[i] = d;
    }

    for (uint i = 0; i < header->deferredBindings; i++) {
        quint32 objectIndex = 0;
        if (!readData((char *)&objectIndex, sizeof (quint32), stream))
            return false;
        quint32 len = 0;
        if (!readData((char *)&len, sizeof (quint32), stream))
            return false;
        if (len > QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH)
            return false;
        QBitArray bindings;
        bindings.resize(len);
        if (!readBitArray(bindings, stream))
            return false;
        deferredBindings.insert(objectIndex, bindings);

    }

#if 0
    // script references
    for (int i = 0; i < (int)header->scriptReferences; i++) {
        QQmlTypeData::ScriptReference scriptReference;
        if (!readData((char *)&scriptReference, sizeof (QQmlTypeData::ScriptReference), stream))
            return false;
        //scriptReferences.append(scriptReference);
    }
#endif

    return true;
}

bool QmcUnit::readBitArray(QBitArray &bitArray, QDataStream &stream)
{
    int len = QMC_UNIT_BIT_ARRAY_LENGTH(bitArray.size());
    quint32 *buf = new quint32[len];
    if (!buf)
        return false;
    if (!readData((char *)buf, len * sizeof(quint32), stream)) {
        delete buf;
        return false;
    }

    quint32 *p = buf;
    for (int i = 0; i < bitArray.size(); i++) {
        if (i % 32 == 0 && i > 0) {
            p++;
        }
        if ((*p >> (i % 32)) & 1)
            bitArray.setBit(i);
    }
    delete buf;
    return true;
}

bool QmcUnit::readString(QString &string, QDataStream &stream)
{
    quint32 stringLen = 0;
    char buf[QMC_UNIT_STRING_MAX_LEN];
    int ret = stream.readRawData((char*)&stringLen, sizeof(quint32));
    if (ret != sizeof(quint32))
        return false;
    if (ret == 0)
        return true;
    if (ret > QMC_UNIT_STRING_MAX_LEN)
        return false;
    ret = stream.readRawData(buf, stringLen);
    if (ret != (int)stringLen)
        return false;
    string.append(QString::fromUtf8(buf, stringLen));
    return true;
}

bool QmcUnit::readData(char *data, int len, QDataStream &stream)
{
    if (stream.readRawData(data, len) != len)
        return false;
    else
        return true;
}

bool QmcUnit::checkHeader(QmcUnitHeader *header)
{
    if (header->type != QMC_QML && header->type != QMC_JS)
        return false;

    if (header->version != QMC_UNIT_VERSION || strncmp(QMC_UNIT_MAGIC_STR, header->magic, strlen(QMC_UNIT_MAGIC_STR)))
        return false;

    if (header->sizeQmlUnit > QMC_UNIT_MAX_QML_UNIT_SIZE || header->sizeQmlUnit < sizeof (QV4::CompiledData::QmlUnit))
        return false;

    if (header->sizeUnit > QMC_UNIT_MAX_COMPILATION_UNIT_SIZE || header->sizeUnit < sizeof (QV4::CompiledData::Unit))
        return false;

    if (header->imports > QMC_UNIT_MAX_IMPORTS)
        return false;

    if (header->strings > QMC_UNIT_MAX_STRINGS)
        return false;

    if (header->namespaces > QMC_UNIT_MAX_NAMESPACES)
        return false;

    if (header->typeReferences > QMC_UNIT_MAX_TYPE_REFERENCES)
        return false;

    if (header->codeRefs > QMC_UNIT_MAX_CODE_REFS)
        return false;

    if (header->constantVectors > QMC_UNIT_MAX_CONSTANT_VECTORS)
        return false;

    if (header->objectIndexToIdRoot > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT)
        return false;

    if (header->objectIndexToIdComponent > QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT)
        return false;

    if (header->aliases > QMC_UNIT_MAX_ALIASES)
        return false;

    if (header->customParsers > QMC_UNIT_MAX_CUSTOM_PARSERS)
        return false;

    if (header->customParserBindings > QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS)
        return false;

    if (header->deferredBindings > QMC_UNIT_MAX_DEFERRED_BINDINGS)
        return false;

    return true;
}

QString QmcUnit::stringAt(int index) const
{
    Q_ASSERT(index < strings.size());
    return strings.at(index);
}

bool QmcUnit::makeExecutable()
{
    foreach (const JSC::MacroAssemblerCodeRef &codeRef, compilationUnit->codeRefs) {
        //JSC::ExecutableAllocator::makeExecutable(code, codeRefLen);

        size_t pageSize = PAGE_SIZE;
        size_t iaddr = reinterpret_cast<size_t>(codeRef.code().executableAddress());
        size_t roundAddr = iaddr & ~(pageSize - static_cast<size_t>(1));
        size_t len = codeRef.size() + (iaddr - roundAddr);
        if (len == 0)
            len++;
        int mode = PROT_READ | PROT_EXEC;
#if 0
        QString roundAddrAsHex = QString("%1").arg(roundAddr, 0, 16);
        QString codeAddrAsHex = QString("%1").arg((size_t)codeRef.code().executableAddress(), 0, 16);
        qDebug() << "set executable addr" << roundAddrAsHex << codeAddrAsHex << "len" << len << "mode" << mode;
#endif
        if (mprotect(reinterpret_cast<void *>(roundAddr), len, mode))
            return false;
    }
    return true;
}

QT_END_NAMESPACE
