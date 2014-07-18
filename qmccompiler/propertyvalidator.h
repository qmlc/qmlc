/*!
 * Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 * Contact: http://www.qt-project.org/legal
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

#ifndef PROPERTYVALIDATOR_H
#define PROPERTYVALIDATOR_H

#include <QString>
#include <QHash>
#include <QBitArray>
#include <QVector>

#include <private/qqmlcustomparser_p.h>
#include <private/qqmlimport_p.h>
#include <private/qqmlbinding_p.h>
#include <private/qqmlcompiler_p.h>

class QQmlEnginePrivate;
class QQmlPropertyCache;
class QQmlPropertyData;

class QmcTypeCompiler;
namespace QV4 {
namespace CompiledData {
struct Binding;
struct QmlUnit;
}
}

class PropertyValidator : public QQmlCustomParserCompilerBackend
{
public:
    PropertyValidator(QmcTypeCompiler *typeCompiler);

    bool validate();

    // Re-implemented for QQmlCustomParser
    virtual const QQmlImports &imports() const;
    virtual QQmlBinding::Identifier bindingIdentifier(const QV4::CompiledData::Binding *binding, QQmlCustomParser *parser);
    virtual QString bindingAsString(int objectIndex, const QV4::CompiledData::Binding *binding) const;

private:
    bool validateObject(int objectIndex, const QV4::CompiledData::Binding *instantiatingBinding, bool populatingValueTypeGroupProperty = false);
    bool validateLiteralBinding(QQmlPropertyCache *propertyCache, QQmlPropertyData *property, const QV4::CompiledData::Binding *binding);
    bool validateObjectBinding(QQmlPropertyData *property, const QString &propertyName, const QV4::CompiledData::Binding *binding);

    bool isComponent(int objectIndex) const { return objectIndexToIdPerComponent.contains(objectIndex); }

    bool canCoerce(int to, QQmlPropertyCache *fromMo);
    QString stringAt(int idx) const;
    void recordError(const QV4::CompiledData::Location &location, const QString &message);

    QmcTypeCompiler *compiler;
    QQmlEnginePrivate *enginePrivate;
    const QV4::CompiledData::QmlUnit *qmlUnit;
    const QHash<int, QQmlCompiledData::TypeReference*> &resolvedTypes;
    const QHash<int, QQmlCustomParser*> &customParsers;
    const QVector<QQmlPropertyCache *> &propertyCaches;
    const QHash<int, QHash<int, int> > objectIndexToIdPerComponent;
    QHash<int, QQmlCompiledData::CustomParserData> *customParserData;
    QVector<int> customParserBindings;
    QHash<int, QBitArray> deferredBindingsPerObject;

    bool _seenObjectWithId;
};

#endif // PROPERTYVALIDATOR_H
