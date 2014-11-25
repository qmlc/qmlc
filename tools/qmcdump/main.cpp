/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Copyright (C) 2014 Nomovok Ltd. All rights reserved.
** Contact: info@nomovok.com
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGuiApplication>
#include <QQmlComponent>
#include <QQuickView>
#include <QFile>
#include <QDebug>
#include <QTextStream>

#include "qmcunit.h"
#include "qmcloader.h"


QTextStream &operator<<(QTextStream &stream, const QmcUnit &unit);
QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::Unit &unit);
QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::CompilationUnit &unit);
QTextStream &operator<<(QTextStream &stream, const QQmlTypeData::ScriptReference &script);

int main(int argc, char *argv[])
{
    int ret;

    if (argc != 2 || !QFile::exists(argv[1])){
        qWarning() << argv[0] << "filename";
        return 1;
    }

    QGuiApplication app(argc, argv);

    QQuickView view;

    QQmlEngine *engine = view.engine();

    QmcLoader loader(engine);
    QString file(argv[1]);
    QFile f(file);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Could not open file for reading: " << file;
        return 2;
    }
    QDataStream in(&f);
    QmcUnit *unit = QmcUnit::loadUnit(in, engine, &loader, QUrl(argv[1]));
    f.close();

    if (unit == NULL) {
         qWarning() << "Could not load QmcUnit object: " << file;
         return 3;
    }

    QTextStream out(stdout);
    out << *unit;
    
    delete unit;
    return 0;
}


QTextStream &operator<<(QTextStream &stream, const QmcUnit &unit)
{
    // QV4::CompiledData::QmlUnit dump.
    stream << "QmlUnit:\n";
    stream << unit.qmlUnit->header;
    stream << " qmlUnitSize " << unit.qmlUnit->qmlUnitSize << "\n";
    stream << " nImports " << unit.qmlUnit->nImports << "\n";
    stream << " offsetToImports " << unit.qmlUnit->offsetToImports << "\n";
    stream << " nObjects " << unit.qmlUnit->nObjects << "\n";
    stream << " offsetToObjects " << unit.qmlUnit->offsetToObjects << "\n";
    stream << " indexOfRootObject " << unit.qmlUnit->indexOfRootObject << "\n";
    stream << *unit.compilationUnit;
    stream << "Strings:\n";
    foreach (const QString &s, unit.strings)
        stream << " " << s << "\n";
    stream << "Imports:\n";
    foreach (const QV4::CompiledData::Import &import, unit.imports)
        stream << unit.strings[import.uriIndex] << " " << unit.strings[import.qualifierIndex] << "\n";
    stream << "Namespaces:\n";
    foreach (const QString &s, unit.namespaces)
        stream << s << "\n";
    stream << "Type references:\n";
    foreach (const QmcUnitTypeReference& r, unit.typeReferences)
        stream << "index: " << r.index
            << " syntheticComponent: " << r.syntheticComponent
            << " composite: " << r.composite << "\n";
    for (int k = 0; k < unit.codeRefData.size(); ++k) {
        stream << "Code ref " << k << "\n";
        stream << "codeRefData:\n";
        int count = 0;
        foreach (char c, unit.codeRefData[k]) {
            if (count % 8 == 0) stream << QString("%1: ").arg(count, 4);
            QString hexVal = QString("%1 ").arg((unsigned char)c, 2, 16);
            stream << hexVal;
            ++count;
            if (count % 8 == 0) stream << "\n";
        }
        if ((count - 1) & 7) stream << "\n";
        stream << "linkCalls:\n";
        foreach (const QmcUnitCodeRefLinkCall &r, unit.linkCalls[k])
            stream << "index: " << r.index
                << " offset: " << r.offset << "\n";
        stream << "constantVectors:\n";
        foreach (const QV4::Primitive &p, unit.constantVectors[k]) {
            QV4::Value v = p.asValue();
            if (v.isEmpty()) stream << "Empty\n";
            else if (v.isNull()) stream << "NULL\n";
            else if (v.isBoolean()) stream << v.booleanValue() << "\n";
            else if (v.isInteger()) stream << v.integerValue() << "\n";
            else if (v.isDouble()) stream << v.doubleValue() << "\n";
            else if (v.isString()) stream << v.stringValue() << "\n";
            else stream << "Object or managed\n";
        }
    }
    stream << "QmcUnitObjectIndexToId:\n";
    foreach (const QmcUnitObjectIndexToId &tmp, unit.objectIndexToIdRoot) {
        stream << "index: " << tmp.index
            << " id: " << tmp.id << "\n";
    }
    stream << "QmcUnitObjectIndexToIdComponent (QmcUnitObjectIndexToId):\n";
    foreach (const QmcUnitObjectIndexToIdComponent &tmp, unit.objectIndexToIdComponent) {
        stream << tmp.componentIndex;
        foreach (const QmcUnitObjectIndexToId &m, tmp.mappings) {
            stream << " (" << m.index << ", " << m.id << ")";
        }
        stream << "\n";
    }
    stream << "QmcUnitAlias:\n";
    foreach (const QmcUnitAlias &tmp, unit.aliases) {
        stream << "objectIndex: " << tmp.objectIndex
            << " propertyIndex: " << tmp.propertyIndex
            << " contextIndex: " << tmp.contextIndex
            << " targetObjectIndex: " << tmp.targetObjectIndex
            << " targetPropertyIndex: " << tmp.targetPropertyIndex
            << " propertyType: " << tmp.propertyType
            << " flags: " << tmp.flags
            << " notifySignal: " << tmp.notifySignal << "\n";
    }
    return stream;
}

QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::Unit &unit)
{
    stream << "Unit:" << "\n";
    stream << " magic[8] " << unit.magic << "\n";
    stream << " architecture " << unit.architecture << "\n";
    stream << " version " << unit.version << "\n";
    stream << " unitSize " << unit.unitSize << "\n";
    stream << " flags " << unit.flags << "\n";
    stream << " stringTableSize " << unit.stringTableSize << "\n";
    stream << " offsetToStringTable " << unit.offsetToStringTable << "\n";
    stream << " functionTableSize " << unit.functionTableSize << "\n";
    stream << " offsetToFunctionTable " << unit.offsetToFunctionTable << "\n";
    stream << " lookupTableSize " << unit.lookupTableSize << "\n";
    stream << " offsetToLookupTable " << unit.offsetToLookupTable << "\n";
    stream << " regexpTableSize " << unit.regexpTableSize << "\n";
    stream << " offsetToRegexpTable " << unit.offsetToRegexpTable << "\n";
    stream << " constantTableSize " << unit.constantTableSize << "\n";
    stream << " offsetToConstantTable " << unit.offsetToConstantTable << "\n";
    stream << " jsClassTableSize " << unit.jsClassTableSize << "\n";
    stream << " offsetToJSClassTable " << unit.offsetToJSClassTable << "\n";
    stream << " indexOfRootFunction " << unit.indexOfRootFunction << "\n";
    stream << " sourceFileIndex " << unit.sourceFileIndex << "\n";
    return stream;
}

QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::CompilationUnit &unit)
{
    stream << "CompilationUnit:\n";
    //stream << *unit.data; Same as previous unit so no need to print.
    stream << " data->functionTableSize: " << unit.data->functionTableSize << "\n";
    for (int k = 0; k < unit.data->functionTableSize; ++k) {
        const QV4::CompiledData::Function *f = unit.data->functionAt(k);
        stream << " index " << f->index << "\n";
        stream << "  nameIndex " << f->nameIndex << "\n";
        stream << "  flags " << f->flags << "\n";
        stream << "  nFormals " << f->nFormals << "\n";
        stream << "  formalsOffset " << f->formalsOffset << "\n";
        stream << "  nLocals " << f->nLocals << "\n";
        stream << "  localsOffset " << f->localsOffset << "\n";
        stream << "  nInnerFunctions " << f->nInnerFunctions << "\n";
        stream << "  innerFunctionsOffset " << f->innerFunctionsOffset << "\n";
        stream << "  location (row, col) " << f->location.line << ", " << f->location.column << "\n";
    }
    return stream;
}

QTextStream &operator<<(QTextStream &stream, const QQmlTypeData::ScriptReference &script)
{
    stream << "ScriptReference:\n";
    stream << " location: " << script.location.line << " " << script.location.column << "\n";
    stream << " qualifier: " << script.qualifier << "\n";
    stream << " script: " << script.script << "\n";
    return stream;
}

