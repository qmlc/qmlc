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

#include <QQmlFile>
#include <private/qobject_p.h>
#include <private/qqmlengine_p.h>

#include "compiler.h"
#include "qmlcompilation.h"
#include "qmcexporter.h"

class CompilerPrivate : QObjectPrivate
{
    Q_DECLARE_PUBLIC(Compiler)

public:
    CompilerPrivate();
    virtual ~CompilerPrivate();

    QList<QQmlError> errors;
    QmlCompilation* compilation;
    QQmlEngine *engine;
    QString basePath;
    bool basePathSet;
};

CompilerPrivate::CompilerPrivate()
    : compilation(NULL),
      basePathSet(false)
{
}

CompilerPrivate::~CompilerPrivate()
{
    delete compilation;
}

Compiler::Compiler(QQmlEngine *engine, QObject *parent) :
    QObject(*(new CompilerPrivate), parent)
{
    Q_D(Compiler);
    d->engine = engine;
}

Compiler::~Compiler()
{
}

QQmlEngine* Compiler::engine()
{
    Q_D(Compiler);
    return d->engine;
}

QmlCompilation* Compiler::takeCompilation()
{
    Q_D(Compiler);
    QmlCompilation* c = d->compilation;
    d->compilation = NULL;
    return c;
}

void Compiler::unsetBasePath()
{
    Q_D(Compiler);
    d->basePathSet = false;
}

void Compiler::setBasePath(const QString &path)
{
    Q_D(Compiler);
    d->basePathSet = true;
    d->basePath = path;
}

bool Compiler::loadData()
{
    Q_D(Compiler);
    const QUrl& url = d->compilation->loadUrl;
    if (!url.isValid() || url.isEmpty())
        return false;
    QQmlFile f;
    f.load(d->compilation->engine, url);
    if (!f.isReady()) {
        if (f.isError()) {
            QQmlError error;
            error.setUrl(url);
            error.setDescription(f.error());
            appendError(error);
        }
        return false;
    }
    d->compilation->code = QString::fromUtf8(f.data());
    return true;
}

void Compiler::clearError()
{
    Q_D(Compiler);
    d->errors.clear();
}

QList<QQmlError> Compiler::errors() const
{
    const Q_D(Compiler);
    return d->errors;
}

void Compiler::appendError(const QQmlError &error)
{
    Q_D(Compiler);
    d->errors.append(error);
}

void Compiler::appendErrors(const QList<QQmlError> &errors)
{
    Q_D(Compiler);
    d->errors.append(errors);
}

QmlCompilation* Compiler::compilation()
{
    Q_D(Compiler);
    return d->compilation;
}

const QmlCompilation* Compiler::compilation() const
{
    const Q_D(Compiler);
    return d->compilation;
}

bool Compiler::compile(const QString &url)
{
    Q_D(Compiler);
    clearError();

    // check that engine is using correct factory
    if (!qgetenv("QV4_FORCE_INTERPRETER").isEmpty()) {
        QQmlError error;
        error.setDescription("Compiler is forced to use interpreter");
        appendError(error);
        return false;
    }

    Q_ASSERT(d->compilation == NULL);
    QmlCompilation* c = new QmlCompilation(url, QUrl(url), d->engine);
    d->compilation = c;
    c->importCache = new QQmlImports(&QQmlEnginePrivate::get(d->compilation->engine)->typeLoader);
    c->importDatabase = new QQmlImportDatabase(d->compilation->engine);
    c->loadUrl = url;
    int lastSlash = url.lastIndexOf('/');
    if (lastSlash == -1)
        c->url = url;
    else if (lastSlash + 1 < url.length())
        c->url = url.mid(lastSlash + 1);
    else
        c->url = "";

    if (!loadData()) {
        delete takeCompilation();
        return false;
    }

    if (!compileData()) {
        delete takeCompilation();
        return false;
    }

    return true;
}

bool Compiler::compile(const QString &url, QDataStream &output)
{
    Q_D(Compiler);
    bool ret = compile(url);
    if (ret) {

        ret = createExportStructures();
        if (d->basePathSet) {
            QString newUrl = d->basePath;
            int lastSlash = url.lastIndexOf('/');
            if (lastSlash == -1)
                newUrl.append(url);
            else if (lastSlash < url.length() - 1)
                newUrl.append(url.mid(lastSlash + 1));
            QUrl url;
            if (newUrl.startsWith(":/"))
                url.setUrl("qrc:" + newUrl);
            else
                url.setUrl("file:" + newUrl);
            d->compilation->url = url;
            d->compilation->urlString = url.toString();
        }
        if (ret)
            ret = exportData(output);
    }

    delete takeCompilation();

    return ret;
}

bool Compiler::compile(const QString &url, const QString &outputFile)
{
    // open output file
    QFile f(outputFile);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        QQmlError error;
        error.setDescription("Could not open file for writing");
        error.setUrl(QUrl(outputFile));
        return false;
    }
    QDataStream out(&f);

    bool ret = compile(url, out);
    f.close();

    return ret;
}

bool Compiler::exportData(QDataStream &output)
{
    Q_D(Compiler);

    if (!d->compilation->checkData()) {
        QQmlError error;
        error.setDescription("Compiled data not valid. Internal error.");
        appendError(error);
        return false;
    }

    QmcExporter exporter(d->compilation);
    bool ret = exporter.exportQmc(output);
    if (!ret) {
        QQmlError error;
        error.setDescription("Error saving data");
        appendError(error);
    }
    return ret;
}

QString Compiler::stringAt(int index) const
{
    const Q_D(Compiler);
    return d->compilation->document->jsGenerator.stringTable.stringForIndex(index);
}

bool Compiler::addImport(const QV4::CompiledData::Import *import, QList<QQmlError> *errors)
{
    Q_D(Compiler);
    const QString &importUri = stringAt(import->uriIndex);
    const QString &importQualifier = stringAt(import->qualifierIndex);

    if (import->type == QV4::CompiledData::Import::ImportScript) {
        // TBD: qqmltypeloader.cpp:1320
        QmlCompilation::ScriptReference scriptRef;
        scriptRef.location = import->location;
        scriptRef.qualifier = importQualifier;
        scriptRef.compilation = NULL;
        d->compilation->scripts.append(scriptRef);
    } else if (import->type == QV4::CompiledData::Import::ImportLibrary) {
        QString qmldirFilePath;
        QString qmldirUrl;
        if (QQmlMetaType::isLockedModule(importUri, import->majorVersion)) {
            //Locked modules are checked first, to save on filesystem checks
            if (!d->compilation->importCache->addLibraryImport(d->compilation->importDatabase, importUri, importQualifier, import->majorVersion,
                                          import->minorVersion, QString(), QString(), false, errors))
                return false;

        } else if (d->compilation->importCache->locateQmldir(d->compilation->importDatabase, importUri, import->majorVersion, import->minorVersion,
                                 &qmldirFilePath, &qmldirUrl)) {
            // This is a local library import
            if (!d->compilation->importCache->addLibraryImport(d->compilation->importDatabase, importUri, importQualifier, import->majorVersion,
                                          import->minorVersion, qmldirFilePath, qmldirUrl, false, errors))
                return false;

            if (!importQualifier.isEmpty()) {
                // Does this library contain any qualified scripts?
                QUrl libraryUrl(qmldirUrl);
                QQmlTypeLoader* typeLoader = &QQmlEnginePrivate::get(d->compilation->engine)->typeLoader;
                const QQmlTypeLoader::QmldirContent *qmldir = typeLoader->qmldirContent(qmldirFilePath, qmldirUrl);
                foreach (const QQmlDirParser::Script &script, qmldir->scripts()) {
                    // TBD: qqmltypeloader.cpp:1343
                    qDebug() << "Library contains scripts";
                    QQmlError error;
                    error.setDescription("Libraries with scripts not supported");
                    appendError(error);
                    return false;
                }
            }
        } else {
            // Is this a module?
            if (QQmlMetaType::isAnyModule(importUri)) {
                if (!d->compilation->importCache->addLibraryImport(d->compilation->importDatabase, importUri, importQualifier, import->majorVersion,
                                              import->minorVersion, QString(), QString(), false, errors))
                    return false;
            } else {
                qDebug() << "Encountered unresolved import";
                QQmlError error;
                error.setDescription("Unresolved import" + importUri);
                error.setLine(import->location.line);
                error.setColumn(import->location.column);
                error.setUrl(d->compilation->url);
                appendError(error);
                return false;
                // TBD: else add to unresolved imports qqmltypeloader.cpp:1356
            }
        }
    } else {
        // qqmltypeloader.cpp:1383
        Q_ASSERT(import->type == QV4::CompiledData::Import::ImportFile);

        QUrl qmldirUrl;
        if (importQualifier.isEmpty()) {
            qmldirUrl = compilation()->loadUrl.resolved(QUrl(importUri + QLatin1String("/qmldir")));
            if (!QQmlImports::isLocal(qmldirUrl)) {
                qDebug() << "File import from network not supported";
                QQmlError error;
                error.setDescription("File import from network not supported");
                errors->append(error);
                return false;
            }
        }

        if (!compilation()->importCache->addFileImport(compilation()->importDatabase, importUri, importQualifier, import->majorVersion,
                                   import->minorVersion, /*incomplete*/ false, errors))
            return false;
    }
    return true;
}
