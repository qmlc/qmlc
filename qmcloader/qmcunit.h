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
 *
 * Author: Mikko Hurskainen <mikko.hurskainen@nomovok.com>
 */

#ifndef QMCUNIT_H
#define QMCUNIT_H

#include <QObject>
#include <QDataStream>
#include <QBitArray>

#include <private/qqmltypeloader_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qqmlcomponent_p.h>
#include <private/qqmltypenamecache_p.h>
#include <private/qv4assembler_p.h>

#include "qmcfile.h"

QT_BEGIN_NAMESPACE

class QmcUnitPropertyCacheCreator;
class QmcTypeUnit;
class QmcLoader;

struct QmcUnit
{
    static QmcUnit *loadUnit(QDataStream &stream, QQmlEngine *engine, QmcLoader *loader, const QUrl &loadedUrl);
    virtual ~QmcUnit();

    QString stringAt(int) const;

    // common data
    QQmlEngine *engine;

    // data from qmc unit file
    QList<QV4::CompiledData::Import> imports;
    QmcUnitHeader *header;
    QV4::CompiledData::QmlUnit* qmlUnit;
    QV4::CompiledData::Unit* unit;
    QV4::JIT::CompilationUnit *compilationUnit;
    QList<QVector<char> > codeRefData;
    QList<QVector<QmcUnitCodeRefLinkCall> > linkCalls;
    QList<QVector<QV4::Primitive> > constantVectors;
    QList<QmcUnitTypeReference> typeReferences;
    QmcUnitExceptionReturnLabel exceptionReturnLabel;
    QList<QmcUnitExceptionPropagationJump> exceptionPropagationJumps;
#if CPU(ARM_THUMB2)
    QList<QmcUnitLinkRecord> linkRecords;
#endif
    QUrl url;
    QString urlString;
    QUrl loadedUrl;
    QList<QString> strings;
    QList<QString> namespaces;
    QList<QV4::ExecutableAllocator::Allocation *> allocations;
    QList<QQmlTypeData::ScriptReference> scripts;
    QmcFileType type;
    QmcLoader* loader;
    QQmlTypeLoader::Blob *blob; // cast to QmcTypeUnit or QmcScriptUnit
    QList<QQmlError> errors;
    QString name;
    QList<QmcUnitAlias> aliases;
    QList<QmcUnitObjectIndexToId> objectIndexToIdRoot;
    QList<QmcUnitObjectIndexToIdComponent> objectIndexToIdComponent;

    QHash<int, QQmlCompiledData::CustomParserData> customParsers;
    QVector<int> customParserBindings;
    QHash<int, QBitArray> deferredBindings;
    QVector<int> codeRefSizes;

private:
    QmcUnit(QmcUnitHeader *header, const QUrl &url, const QString &urlString, QQmlEngine *engine, QmcLoader *loader, const QString &name, const QUrl &loadedUrl);
    bool loadUnitData(QDataStream &stream);
    static bool checkHeader(QmcUnitHeader *header);
    static bool readString(QString &string, QDataStream &stream);
    static bool readData(char *data, int len, QDataStream &stream);
    static bool readBitArray(QBitArray &bitArray, QDataStream &stream);
};

QT_END_NAMESPACE

#endif // QMCUNIT_H
