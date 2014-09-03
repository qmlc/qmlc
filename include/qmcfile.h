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

#ifndef QMCFILE_H
#define QMCFILE_H

#include <QString>
#include <QUrl>
#include <QBitArray>
#include <QByteArray>

#include <private/qv4compileddata_p.h>

static const char QMC_UNIT_MAGIC_STR[] = "qmcunit1";

#define QMC_UNIT_STRING_MAX_LEN 16384

#define QMC_UNIT_VERSION 1

#define QMC_UNIT_NAME_MAX_LEN QMC_UNIT_STRING_MAX_LEN

#define QMC_UNIT_URL_MAX_LEN QMC_UNIT_STRING_MAX_LEN

#define QMC_UNIT_MAX_QML_UNIT_SIZE 65536

#define QMC_UNIT_MAX_COMPILATION_UNIT_SIZE 65536

#define QMC_UNIT_MAX_IMPORTS 256

#define QMC_UNIT_MAX_STRINGS 2048

#define QMC_UNIT_MAX_NAMESPACES 256

#define QMC_UNIT_MAX_TYPE_REFERENCES 256

#define QMC_UNIT_MAX_CODE_REFS 256
#define QMC_UNIT_MAX_CODE_REF_SIZE 65536

#define QMC_UNIT_MAX_CODE_REF_LINK_CALLS 256

#define QMC_UNIT_MAX_CONSTANT_VECTORS 256
#define QMC_UNIT_MAX_CONSTANT_VECTOR_SIZE 256

#define QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_ROOT 256

#define QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT 256
#define QMC_UNIT_MAX_OBJECT_INDEX_TO_ID_COMPONENT_MAPPINGS 256

#define QMC_UNIT_MAX_ALIASES 256

#define QMC_UNIT_MAX_CUSTOM_PARSERS 256
#define QMC_UNIT_MAX_CUSTOM_PARSER_DATA_LENGTH 2048
#define QMC_UNIT_MAX_CUSTOM_PARSER_BINDING_LENGTH 32

#define QMC_UNIT_MAX_CUSTOM_PARSER_BINDINGS 256

#define QMC_UNIT_MAX_DEFERRED_BINDINGS 256
#define QMC_UNIT_MAX_DEFERRED_BINDING_LENGTH 32

#define QMC_UNIT_BIT_ARRAY_LENGTH(x) (((x - 1) >> 5) + 1)

enum QmcFileType {
    QMC_QML = 0,
    QMC_JS
};

struct QmcUnitHeader {
    char magic[8];
    quint32 type;
    qint16 architecture;
    qint16 version;
    quint32 sizeQmlUnit;
    quint32 sizeUnit;
    quint32 imports;
    quint32 strings;
    quint32 namespaces;
    quint32 typeReferences;
    quint32 codeRefs;
    quint32 objectIndexToIdRoot;
    quint32 objectIndexToIdComponent;
    quint32 aliases;
    quint32 customParsers;
    quint32 customParserBindings;
    quint32 deferredBindings;
};

struct QmcUnitTypeReference {
    quint32 index;
    quint32 syntheticComponent;
    quint32 composite;
};

struct QmcUnitObjectIndexToId {
    quint32 index;
    quint32 id;
};

struct QmcUnitObjectIndexToIdComponent {
    quint32 componentIndex;
    QVector<QmcUnitObjectIndexToId> mappings;
};

struct QmcUnitAlias { // need to be sorted by objectIndex, propertyIndex
    quint32 objectIndex;
    quint32 propertyIndex;
    // see QQmlVMEMetaObject::AliasData
    quint32 contextIndex; // contentIdx
    quint32 targetPropertyIndex; // propertyIdx
    quint32 propertyType; // propType
    quint32 flags; // flags
    quint32 notifySignal; // notifySignal
};

struct QmcUnitCustomParser {
    quint32 objectIndex;
    QByteArray compilationArtifact;
    QBitArray bindings;
};

struct QmcUnitDeferredBinding {
    quint32 objectIndex;
    QBitArray bindings;
};

struct QmcUnitCodeRefLinkCall {
    quint32 index; // as in qmclinktable.h
    quint32 offset; // inside coderef
};

#endif // QMCFILE_H
