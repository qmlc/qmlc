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

#ifndef QMCTYPEUNIT_H
#define QMCTYPEUNIT_H

#include <private/qqmltypeloader_p.h>

class QmcUnit;
class QmcUnitPropertyCacheCreator;
class QmcTypeUnitComponentAndAliasResolver;

class QmcTypeUnit : public QQmlTypeLoader::Blob
{
public:
    QmcTypeUnit(QmcUnit *unit, QQmlTypeLoader *typeLoader);
    virtual ~QmcTypeUnit();

    bool link();
    QQmlComponent *createComponent();
    QmcUnit *qmcUnit();
    QQmlCompiledData *refCompiledData();

    // qqmltypeloader_p.h:540
    struct ScriptReference
    {
        ScriptReference() : script(0) {}

        QV4::CompiledData::Location location;
        QString qualifier;
        QString nameSpace;
        QQmlScriptBlob *script;
    };

protected:
    virtual void dataReceived(const Data &);
    virtual void initializeFromCachedUnit(const QQmlPrivate::CachedQmlUnit*);
    virtual QString stringAt(int) const;
    virtual void done();

    virtual void scriptImported(QQmlScriptBlob *blob, const QV4::CompiledData::Location &location, const QString &qualifier, const QString &nameSpace);

private:
    bool addImports();
    bool initDependencies();
    bool initQml();
    bool sourceNameForUrl(const QUrl &url, QString &name);

    QmcUnit *unit;

    // created for type unit
    QQmlCompiledData *compiledData;
    bool linked;
    QVector<QByteArray>& vmeMetaObjects;
    QVector<QQmlPropertyCache*>& propertyCaches;
    //QList<QQmlTypeData::TypeReference> compositeSingletons;
    bool doneLinking;
    QList<ScriptReference> scripts;
    QList<QmcUnit *> dependencies;
    friend class QmcUnitPropertyCacheCreator;
    friend class QmcTypeUnitComponentAndAliasResolver;
};

#endif // QMCTYPEUNIT_H
