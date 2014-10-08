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

#include "qmcexporter.h"
#include "qmlcompilation.h"

#include <private/qv4assembler_p.h>
#include <private/qqmlcompiler_p.h>

QmcExporter::QmcExporter(QmlCompilation *compilation, QObject *parent) :
    QObject(parent),
    compilation(compilation)
{
}

bool QmcExporter::exportQmc(QDataStream &stream)
{
    return writeQmcUnit(compilation, stream);
}

void QmcExporter::createHeader(QmcUnitHeader &header, QmlCompilation *c)
{
    memset(&header, 0, sizeof(QmcUnitHeader));
    strcpy(header.magic, QMC_UNIT_MAGIC_STR);
    header.architecture = 0;
    header.version = QMC_UNIT_VERSION;
    header.type = (quint32)c->type;
    header.sizeQmlUnit = c->qmlUnit->qmlUnitSize;
    header.sizeUnit = c->unit->data->unitSize;
    header.imports = c->qmlUnit->nImports;
    header.strings = c->unit->data->stringTableSize;
    header.namespaces = c->namespaces.size();
    header.typeReferences = c->exportTypeRefs.size();
    QV4::JIT::CompilationUnit* compilationUnit = static_cast<QV4::JIT::CompilationUnit *>(c->unit);
    header.codeRefs = compilationUnit->codeRefs.size();
    header.objectIndexToIdRoot = c->objectIndexToIdRoot.size();
    header.objectIndexToIdComponent = c->objectIndexToIdComponent.size();
    header.aliases = c->aliases.size();
    header.customParsers = c->customParsers.size();
    header.customParserBindings = c->customParserBindings.size();
    header.deferredBindings = c->deferredBindings.size();
}

bool QmcExporter::writeBitArray(QDataStream &stream, const QBitArray &array)
{
    quint32 lenBits = array.size();
    quint32 len = QMC_UNIT_BIT_ARRAY_LENGTH(array.size());
    if (!writeData(stream, (const char *)&lenBits, sizeof(quint32)))
        return false;
    if (array.size() == 0)
        return true;
    quint32* buf = new quint32[len];
    quint32* p = buf;
    for (int i = 0; i < array.size(); i++) {
        if (i % 32 == 0) {
            *p = 0;
            if (i > 0)
                p++;
        }
        if (array.at(i)) {
            if (i % 32 == 0)
                *p = 1;
            else
                *p = ( (*p) | (1 << (i % 32)));
        }
    }
    if (!writeData(stream, (const char *)buf, sizeof (quint32) * len))
        return false;
    return true;
}

bool QmcExporter::writeString(QDataStream &stream, QString string)
{
    quint32 len = string.length();
    int ret = stream.writeRawData((const char*)&len, sizeof(len));
    if (ret != sizeof(len))
        return false;

    if (len == 0)
        return true;

    ret = stream.writeRawData(string.toUtf8().data(), string.length());
    if (ret != string.length())
        return false;
    return true;
}

bool QmcExporter::writeData(QDataStream& stream, const char* data, int len)
{
    if (stream.writeRawData(data, len) != len)
        return false;
    else
        return true;
}

bool QmcExporter::writeDataWithLen(QDataStream& stream, const char* data, int len)
{
    quint32 l = len;
    if (!writeData(stream, (const char *)&l, sizeof(quint32)))
        return false;
    if (!writeData(stream, data, len))
        return false;
    return true;
}

bool QmcExporter::writeQmcUnit(QmlCompilation *c, QDataStream &stream)
{
    QmcUnitHeader header;
    createHeader(header, c);

    if (!writeData(stream, (const char*)&header, sizeof (QmcUnitHeader)))
        return false;

    if (!writeString(stream, c->name))
        return false;

    if (!writeString(stream, c->urlString))
        return false;

    QV4::CompiledData::QmlUnit *qmlUnit = c->qmlUnit;
    if (!writeData(stream, (const char*)qmlUnit, qmlUnit->qmlUnitSize))
        return false;
    QV4::CompiledData::Unit *unit = c->unit->data;
    if (!writeData(stream, (const char*)unit, unit->unitSize))
        return false;

#if 0
    for (uint i = 0; i < qmlUnit->nObjects; i++) {
        const QV4::CompiledData::Object *obj = qmlUnit->objectAt(i);
        qDebug() << "object" << i << "inh" << obj->inheritedTypeNameIndex << "/" << c->unit->data->stringAt(obj->inheritedTypeNameIndex);
        for (uint j = 0; j < obj->nBindings; j++) {
            const QV4::CompiledData::Binding *b = &obj->bindingTable()[j];
            qDebug() << "-> binding" << j << "type" << b->type << "recurse" << bool(b->type >= QV4::CompiledData::Binding::Type_Object) << "target" << b->value.objectIndex;
        }
    }
#endif

    // imports
    for (uint i = 0; i < c->qmlUnit->nImports; i++) {
        const QV4::CompiledData::Import *import = c->qmlUnit->importAt(i);
        if (!writeData(stream, (const char*)import, sizeof(QV4::CompiledData::Import)))
            return false;
    }

    // strings
    for (uint i = 0; i < header.strings; i++) {
        if (!writeString(stream, c->unit->data->stringAt(i)))
            return false;
    }

    // namespaces
    foreach (const QString &ns, c->namespaces) {
        if (!writeString(stream, ns))
            return false;
    }

    // type references
    foreach (const QmcUnitTypeReference &typeRef, c->exportTypeRefs) {
        if (!writeData(stream, (const char *)&typeRef, sizeof (QmcUnitTypeReference)))
            return false;
    }

    // codeRefs
    QV4::JIT::CompilationUnit* compilationUnit = static_cast<QV4::JIT::CompilationUnit *>(c->unit);
    for (int i = 0; i < compilationUnit->codeRefs.size(); i++) {

#if CPU(ARM_THUMB2)
        // linkRecords
        QList<QmcUnitLinkRecord> records = c->jumpsToLinkData[i];
        quint32 recordsCount = records.size();
        if (!writeData(stream, (const char *)&recordsCount, sizeof(quint32)))
            return false;
        foreach (const QmcUnitLinkRecord record, records){
            if (!writeData(stream, (const char *)&record, sizeof (QmcUnitLinkRecord)))
                return false;
        }
        // save unlinkedCode in place of the codeRefs because it gets used after
        // linking in the loading stage
        if (!writeDataWithLen(stream, (const char *)c->unlinkedCodeData[i].data.data(), c->unlinkedCodeData[i].size))
            return false;
#else
        const JSC::MacroAssemblerCodeRef &codeRef = compilationUnit->codeRefs[i];
        if (!writeDataWithLen(stream, (const char *)codeRef.code().dataLocation(), codeRef.size()))
            return false;
#endif
        const QVector<QmcUnitCodeRefLinkCall> &linkCalls = c->linkData[i];
        const QVector<QV4::Primitive> &constantValue = compilationUnit->constantValues[i];
        quint32 linkCallCount = linkCalls.size();
        if (!writeData(stream, (const char *)&linkCallCount, sizeof(quint32)))
            return false;
        if (linkCallCount > 0) {
            if (!writeData(stream, (const char *)linkCalls.data(), linkCalls.size() * sizeof (QmcUnitCodeRefLinkCall)))
                return false;
        }
        quint32 constTableCount = constantValue.size();
        if (!writeData(stream, (const char *)&constTableCount, sizeof(quint32)))
            return false;
        if (constTableCount > 0) {
            if (!writeData(stream, (const char*)constantValue.data(), sizeof(QV4::Primitive) * constantValue.size()))
                return false;
        }
    }

    // object index -> id
    foreach (const QmcUnitObjectIndexToId &mapping, c->objectIndexToIdRoot) {
        if (!writeData(stream, (const char *)&mapping, sizeof (QmcUnitObjectIndexToId)))
            return false;
    }

    // component index + object index -> id
    foreach (const QmcUnitObjectIndexToIdComponent &mapping, c->objectIndexToIdComponent) {
        if (!writeData(stream, (const char *)&mapping.componentIndex, sizeof (quint32)))
            return false;
        quint32 len = mapping.mappings.size();
        if (!writeData(stream, (const char *)&len, sizeof (quint32)))
            return false;
        if (mapping.mappings.size() == 0)
            continue;
        if (!writeData(stream, (const char *)mapping.mappings.data(), mapping.mappings.size() * sizeof (QmcUnitObjectIndexToId)))
            return false;
    }

    // aliases
    foreach (const QmcUnitAlias &alias, c->aliases) {
        if (!writeData(stream, (const char *)&alias, sizeof (QmcUnitAlias)))
            return false;
    }

    // custom parsers
    foreach (const QmcUnitCustomParser &customParser, c->customParsers) {
        if (!writeData(stream, (const char *)&customParser.objectIndex, sizeof(quint32)))
            return false;
        if (!writeDataWithLen(stream, (const char *)customParser.compilationArtifact.data(),
                              customParser.compilationArtifact.size()))
            return false;
        if (!writeBitArray(stream, customParser.bindings))
            return false;
    }

    // custom parser bindings
    foreach (int i, c->customParserBindings) {
        quint32 d = (quint32)i;
        if (!writeData(stream, (const char *)&d, sizeof (quint32)))
            return false;
    }

    // deferred bindings
    foreach (const QmcUnitDeferredBinding &deferredBinding, c->deferredBindings) {
        if (!writeData(stream, (const char *)&deferredBinding.objectIndex, sizeof (quint32)))
            return false;
        if (!writeBitArray(stream, deferredBinding.bindings))
            return false;
    }

    return true;
}
