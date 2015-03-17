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
QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::QmlUnit &unit);
QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::CompilationUnit &unit);
QTextStream &operator<<(QTextStream &stream, const QQmlTypeData::ScriptReference &script);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        qWarning() << argv[0] << "filenames";
        return 1;
    }

    QGuiApplication app(argc, argv);

    QQuickView view;

    QQmlEngine *engine = view.engine();

    QmcLoader loader(engine);
    QTextStream out(stdout);
    for (int k = 1; k < argc; ++k) {
        QString file(argv[k]);
        QFile f(file);
        if (!f.open(QFile::ReadOnly)) {
            qWarning() << "Could not open file for reading: " << file;
            if (argc > 2)
                continue;
            return 2;
        }
        QDataStream in(&f);
        QmcUnit *unit = QmcUnit::loadUnit(in, engine, &loader, QUrl(argv[k]));
        f.close();

        if (unit == NULL) {
            qWarning() << "Could not load QmcUnit object: " << file;
            if (argc > 2)
                continue;
            return 3;
        }

        if (argc > 2)
            out << file << ":\n";
        out << *unit;
        if (argc > 2 && k + 1 < argc)
            out << "\n";
        delete unit;
    }
    return 0;
}


QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::Location &loc)
{
    stream << loc.line << ", " << loc.column;
    return stream;
}

static const QList<QString>* strs = NULL;
static void printString(QTextStream &stream, int index)
{
    if (strs)
        stream << " (" << (*strs)[index] << ")";
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
    strs = &(unit.strings);
    stream << *unit.qmlUnit;
    stream << *unit.compilationUnit;
    stream << "Strings:\n";
    int counter = 0;
    foreach (const QString &s, unit.strings)
        stream << " " << counter++ << ": " << s << "\n";
    stream << "Namespaces:\n";
    foreach (const QString &s, unit.namespaces)
        stream << s << "\n";
    stream << "Script references:\n";
    foreach (const QString &s, unit.scriptReferences)
        stream << " " << s << "\n";
    stream << "Type references:\n";
    QStringList types;
    foreach (const QmcUnitTypeReference& r, unit.typeReferences) {
        types << QString(" index: %1 syntheticComponent: %2 composite: %3\n").
            arg(r.index, 2).arg(r.syntheticComponent).arg(r.composite);
    }
    types.sort();
    foreach (const QString& s, types)
        stream << s;
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
            stream << " index: " << r.index
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
    QStringList ids;
    foreach (const QmcUnitObjectIndexToId &tmp, unit.objectIndexToIdRoot) {
        ids << QString(" index: %1 id: %2\n").arg(tmp.index, 2).arg(tmp.id);
    }
    ids.sort();
    foreach (const QString& s, ids)
        stream << s;
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
        stream << " objectIndex: " << tmp.objectIndex
            << "  propertyIndex: " << tmp.propertyIndex
            << "  contextIndex: " << tmp.contextIndex
            << "  targetObjectIndex: " << tmp.targetObjectIndex
            << "  targetPropertyIndex: " << tmp.targetPropertyIndex
            << "  propertyType: " << tmp.propertyType
            << "  flags: " << tmp.flags
            << "  notifySignal: " << tmp.notifySignal << "\n";
    }
    stream << "customParsers: " << unit.customParsers.size() << "\n";
    stream << "customParserBindings: " << unit.customParserBindings.size() << "\n";
    stream << "deferredBindings: " << unit.deferredBindings.size() << "\n";
    stream << "singletonReferences: " << unit.compositeSingletons.size() << "\n";
    foreach (const QmcSingletonTypeReference &ref, unit.compositeSingletons) {
        stream << " typeName: " << ref.typeName
            << " prefix: " << ref.prefix << "\n";
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

QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::QmlUnit &unit)
{
    stream << "QmlUnit:\n";
    stream << " isSingleton: " << unit.isSingleton();
    stream << "\nObjects:" << unit.nObjects;
    for (unsigned k = 0; k < unit.nObjects; ++k) {
        const QV4::CompiledData::Object *obj = unit.objectAt(k);
        stream << "\nObject " << k << ":";
        stream << "\n inheritedTypeNameIndex: " << obj->inheritedTypeNameIndex;
        printString(stream, obj->inheritedTypeNameIndex);
        stream << "\n idIndex: " << obj->idIndex;
        printString(stream, obj->idIndex);
        stream << "\n indexOfDefaultProperty: " << obj->indexOfDefaultProperty;
        stream << "\n nFunctions: " << obj->nFunctions;
        stream << "  offsetToFunctions: " << obj->offsetToFunctions;
        stream << "\n nProperties: " << obj->nProperties;
        stream << "  offsetToProperties: " << obj->offsetToProperties;
        stream << "\n nSignals: " << obj->nSignals;
        stream << "  offsetToSignals: " << obj->offsetToSignals;
        stream << "\n nBindings: " << obj->nBindings;
        stream << "  offsetToBindings: " << obj->offsetToBindings;
        stream << "\n location: " << obj->location;
        stream << "\n locationOfIdProperty: " << obj->locationOfIdProperty;
        stream << "\n Properties: " << obj->nProperties;
        for (unsigned n = 0; n < obj->nProperties; ++n) {
            const QV4::CompiledData::Property &prop = obj->propertyTable()[n];
            stream << "\n  Property " << n << ":";
            stream << "\n   nameIndex: " << prop.nameIndex;
            printString(stream, prop.nameIndex);
            stream << "\n   type: ";
            switch (prop.type) {
            case QV4::CompiledData::Property::Var: stream << "Var"; break;
            case QV4::CompiledData::Property::Variant: stream << "Variant"; break;
            case QV4::CompiledData::Property::Int: stream << "Int"; break;
            case QV4::CompiledData::Property::Bool: stream << "Bool"; break;
            case QV4::CompiledData::Property::Real: stream << "Real"; break;
            case QV4::CompiledData::Property::String: stream << "String"; break;
            case QV4::CompiledData::Property::Url: stream << "Url"; break;
            case QV4::CompiledData::Property::Color: stream << "Color"; break;
            case QV4::CompiledData::Property::Font: stream << "Font"; break;
            case QV4::CompiledData::Property::Time: stream << "Time"; break;
            case QV4::CompiledData::Property::Date: stream << "Date"; break;
            case QV4::CompiledData::Property::DateTime: stream << "DateTime"; break;
            case QV4::CompiledData::Property::Rect: stream << "Rect"; break;
            case QV4::CompiledData::Property::Point: stream << "Point"; break;
            case QV4::CompiledData::Property::Size: stream << "Size"; break;
            case QV4::CompiledData::Property::Vector2D: stream << "Vector2D"; break;
            case QV4::CompiledData::Property::Vector3D: stream << "Vector3D"; break;
            case QV4::CompiledData::Property::Vector4D: stream << "Vector4D"; break;
            case QV4::CompiledData::Property::Matrix4x4: stream << "Matrix4x4"; break;
            case QV4::CompiledData::Property::Quaternion: stream << "Quaternion"; break;
            case QV4::CompiledData::Property::Alias: stream << "Alias\n  aliasIdValueIndex: " << prop.aliasIdValueIndex; break;
            case QV4::CompiledData::Property::Custom: stream << "Custom\n  customTypeNameIndex: " << prop.customTypeNameIndex; break;
            case QV4::CompiledData::Property::CustomList: stream << "CustomList"; break;
            default: stream << "*** Unknown type, error? ***"; break;
            }
            stream << "\n   aliasPropertyValueIndex: " << prop.aliasPropertyValueIndex;
            stream << "\n   flags: " << prop.flags;
            stream << "\n   location: " << prop.location;
            if (prop.type == QV4::CompiledData::Property::Alias)
                stream << "\n   aliasLocation: " << prop.aliasLocation;
        }
        stream << "\n Bindings: " << obj->nBindings;
        for (unsigned n = 0; n < obj->nBindings; ++n) {
            const QV4::CompiledData::Binding &b = obj->bindingTable()[n];
            stream << "\n  Binding " << n << ":";
            stream << "\n   propertyNameIndex: " << b.propertyNameIndex;
            printString(stream, b.propertyNameIndex);
            stream << "\n   flags: (" << b.flags << ")";
            if (b.flags & QV4::CompiledData::Binding::IsSignalHandlerExpression)
                stream << " IsSignalHandlerExpression";
            if (b.flags & QV4::CompiledData::Binding::IsSignalHandlerObject)
                stream << " IsSignalHandlerObject";
            if (b.flags & QV4::CompiledData::Binding::IsOnAssignment)
                stream << " IsOnAssignment";
            if (b.flags & QV4::CompiledData::Binding::InitializerForReadOnlyDeclaration)
                stream << " InitializerForReadOnlyDeclaration";
            if (b.flags & QV4::CompiledData::Binding::IsResolvedEnum)
                stream << " IsResolvedEnum";
            if (b.flags & QV4::CompiledData::Binding::IsListItem)
                stream << " IsListItem";
            if (b.flags & QV4::CompiledData::Binding::IsBindingToAlias)
                stream << " IsBindingToAlias";
            stream << "\n   type: ";
            switch (b.type) {
            case QV4::CompiledData::Binding::Type_Invalid: stream << "Type_Invalid"; break;
            case QV4::CompiledData::Binding::Type_Boolean: stream << "Type_Boolean\n   value: " << b.valueAsBoolean(); break;
            case QV4::CompiledData::Binding::Type_Number: stream << "Type_Number\n   value: " << b.valueAsNumber(); break;
            case QV4::CompiledData::Binding::Type_String: stream << "Type_String\n   stringIndex: "<< b.stringIndex; break;
            case QV4::CompiledData::Binding::Type_Translation:
                stream << "Type_Translation\n   stringIndex: "<< b.stringIndex;
                stream << "\n   value.translationData (commentIndex, number): "                     << b.value.translationData.commentIndex << " "
                    << b.value.translationData.number;
                break;
            case QV4::CompiledData::Binding::Type_TranslationById: stream << "Type_TranslationById"; break;
            case QV4::CompiledData::Binding::Type_Script: stream << "Type_Script\n   stringIndex: "<< b.stringIndex; break;
            case QV4::CompiledData::Binding::Type_Object: stream << "Type_Object"; break;
            case QV4::CompiledData::Binding::Type_AttachedProperty: stream << "Type_AttachedProperty"; break;
            case QV4::CompiledData::Binding::Type_GroupProperty: stream << "Type_GroupProperty"; break;
            default: stream << "*** Unknown type, error? ***"; break;
            }
            stream << "\n   location: " << b.location;
            stream << "\n   valueLocation: " << b.valueLocation;
        }
        stream << "\n Signals: " << obj->nSignals;
        for (unsigned n = 0; n < obj->nSignals; ++n) {
            const QV4::CompiledData::Signal *s = obj->signalAt(n);
            stream << "\n  Signal " << n << ":";
            stream << "\n   nameIndex: " << s->nameIndex;
            stream << "\n   nParameters: " << s->nParameters;
            for (unsigned p = 0; p < s->nParameters; ++p) {
                const QV4::CompiledData::Parameter *par = s->parameterAt(p);
                stream << "\n   Parameter " << p << ":";
                stream << "\n    nameIndex: " << par->nameIndex;
                stream << "\n    type: " << par->type;
                stream << "\n    customTypeNameIndex: " << par->customTypeNameIndex;
                stream << "\n    reserved: " << par->reserved;
                stream << "\n    location: " << par->location;
            }
            stream << "\n   location: " << s->location;
        }
    }
    stream << "\nImports: " << unit.nImports;
    for (unsigned k = 0; k < unit.nImports; ++k) {
        const QV4::CompiledData::Import *imp = unit.importAt(k);
        stream << "\n Import " << k << ":";
        stream << "\n  type: ";
        switch (imp->type) {
        case QV4::CompiledData::Import::ImportLibrary: stream << "ImportLibrary"; break;
        case QV4::CompiledData::Import::ImportFile: stream << "ImportFile"; break;
        case QV4::CompiledData::Import::ImportScript: stream << "ImportScript"; break;
        default: stream << "*** Unknwon type, error? ***"; break;
        }
        stream << "\n  uriIndex: " << imp->uriIndex;
        printString(stream, imp->uriIndex);
        stream << "\n  qualifierIndex: " << imp->qualifierIndex;
        printString(stream, imp->qualifierIndex);
        stream << "\n  majorVersion, minorVersion: " << imp->majorVersion << ", " << imp->minorVersion;
        stream << "\n  location:" << imp->location;
    }
    stream << "\n";
    return stream;
}

QTextStream &operator<<(QTextStream &stream, const QV4::CompiledData::CompilationUnit &unit)
{
    stream << "CompilationUnit:\n";
    //stream << *unit.data; Same as previous unit so no need to print.
    stream << " data->functionTableSize: " << unit.data->functionTableSize << "\n";
    for (unsigned k = 0; k < unit.data->functionTableSize; ++k) {
        const QV4::CompiledData::Function *f = unit.data->functionAt(k);
        stream << " index " << f->index << "\n";
        stream << "  nameIndex " << f->nameIndex;
        printString(stream, f->nameIndex);
        stream << "\n";
        stream << "  flags (" << f->flags << ")";
        if (f->flags & QV4::CompiledData::Function::HasDirectEval)
            stream << " HasDirectEval";
        if (f->flags & QV4::CompiledData::Function::UsesArgumentsObject)
            stream << " UsesArgumentsObject";
        if (f->flags & QV4::CompiledData::Function::IsStrict)
            stream << " IsStrict";
        if (f->flags & QV4::CompiledData::Function::IsNamedExpression)
            stream << " IsNamedExpression";
        if (f->flags & QV4::CompiledData::Function::HasCatchOrWith)
            stream << " HasCatchOrWith";
        stream << "\n";
        stream << "  nFormals " << f->nFormals;
        stream << "  formalsOffset " << f->formalsOffset << "\n";
        stream << "  nLocals " << f->nLocals;
        stream << "  localsOffset " << f->localsOffset << "\n";
        stream << "  nInnerFunctions " << f->nInnerFunctions;
        stream << "  innerFunctionsOffset " << f->innerFunctionsOffset << "\n";
        stream << "  location (row, col) " << f->location << "\n";
    }

    return stream;
}

QTextStream &operator<<(QTextStream &stream, const QQmlTypeData::ScriptReference &script)
{
    stream << "ScriptReference:\n";
    stream << " location: " << script.location << "\n";
    stream << " qualifier: " << script.qualifier << "\n";
    stream << " script: " << script.script << "\n";
    return stream;
}

