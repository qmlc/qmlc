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

#ifndef QMCTYPECOMPILER_H
#define QMCTYPECOMPILER_H

#include <QQmlError>
#include <QHash>
#include <QVector>

#include <private/qqmlpropertycache_p.h>
#include <private/qqmlirbuilder_p.h>
#include <private/qqmlcompiler_p.h>

class QmlCompilation;
class QQmlCompiledData;
class QQmlCustomParser;

#include "qmlcompilation.h"

class QmcTypeCompiler
{
public:
    QmcTypeCompiler(QmlCompilation *compilation);
    bool precompile();
    QList<QQmlError> compilationErrors() const;
    void recordError(const QQmlError& error);
    QmlCompilation* data();
    int rootObjectIndex() const;
    QString stringAt(int idx) const;
    QList<QmlIR::Object*> *qmlObjects();
    void setPropertyCaches(const QVector<QQmlPropertyCache *> &caches);
    void setVMEMetaObjects(const QVector<QByteArray> &metaObjects);
    QHash<int, QQmlCompiledData::TypeReference*>* resolvedTypes();
    void recordError(const QV4::CompiledData::Location& location, const QString& description);
    const QVector<QQmlPropertyCache *>& propertyCaches() const;
    const QHash<int, QQmlCustomParser*>& customParserCache() const;
    const QV4::Compiler::StringTableGenerator* stringPool() const;

    QQmlJS::MemoryPool* memoryPool();
    QStringRef newStringRef(const QString &string);
    int registerString(const QString &str);
    QString bindingAsString(const QmlIR::Object *object, int scriptIndex) const;

    QHash<int, int> *objectIndexToIdForRoot();
    QHash<int, QHash<int, int> > *objectIndexToIdPerComponent();

    QV4::IR::Module *jsIRModule() const;

    QHash<int, QQmlCompiledData::CustomParserData> *customParserData();

    void setCustomParserBindings(const QVector<int> &bindings);
    void setDeferredBindingsPerObject(const QHash<int, QBitArray> &deferredBindingsPerObject);

    QmlCompilation::TypeReference *findTypeRef(int index);

    void setAliasIdToObjectIndex(const QHash<int, int> &idToObjectIndex);
    void appendAliasIdToObjectIndexPerComponent(int index, const QHash<int, int> &idToObjectIndex);

private:
    void indexCustomParserScripts();
    void annotateAliases();
    bool createTypeMap();
    bool createPropertyCacheVmeMetaData();
    void mergeDefaultProperties();
    bool convertSignalHandlersToFunctions();
    bool resolveEnums();
    void scanScriptStrings();
    bool resolveComponentBoundariesAndAliases();
    bool validateProperties();
    void simplifyJavaScriptBindingExpressions();

    QmlCompilation *compilation;
    QQmlCompiledData *compiledData;
    QList<QQmlError> errors;
    QHash<int, QQmlCustomParser *> customParsers;
};

#endif // QMCTYPECOMPILER_H
