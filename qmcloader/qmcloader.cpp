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

#include <QQmlEngine>
#include <QMap>
#include <QDir>

#include <QQmlComponent>

#include <private/qv4isel_moth_p.h>
#include <private/qobject_p.h>
#include <private/qqmlcompiler_p.h>
#include <private/qqmlcomponent_p.h>
#include <private/qv4compileddata_p.h>
#include <private/qv4context_p.h>
#include <private/qv4debugging_p.h>

#include "qmcloader.h"
#include "qmcfile.h"
#include "qmcunit.h"
#include "qmctypeunit.h"
#include "qmcdebugdata.h"
#include "qmcscriptunit.h"

static int DEPENDENCY_MAX_RECURSION_DEPTH = 10;

// Used when not debugging QML. Needed because compiled code may still call it.
QV4::ReturnedValue doNothing(QV4::CallContext *ctx)
{
    Q_UNUSED(ctx)
    return QV4::Encode::undefined();
}

QV4::ReturnedValue checkBreakpoint(QV4::CallContext *ctx)
{
#if defined(DEBUG)
    // Should be redundant as the compiler does the calls. All ok or none ok.
    if (ctx->callData->argc != 2)
        V4THROW_ERROR("checkBreakpoint() requires two arguments");
    if (!ctx->callData->args[0].isInteger())
        V4THROW_ERROR("checkBreakpoint(): first argument (line number) must be an integer");
    if (!ctx->callData->args[1].isInteger())
        V4THROW_ERROR("checkBreakpoint(): second argument (identifier) must be an integer");
    ctx->lineNumber = ctx->callData->args[0].integerValue();
    int sourceId = ctx->callData->args[1].integerValue();
    QTextStream out(stdout);
    out << "Break check: " << ctx->lineNumber << " " << sourceId;
    QmcDebug* debug = debugData(sourceId);
    if (debug) {
        out << " " << debug->sourceName;
    }
    out << "\n";
    out.flush();
#endif
    // ctx is accessible via engine so let QmcDebugger get the call args.
    ctx->engine->debugger->maybeBreakAtInstruction();
    return QV4::Encode::undefined();
}

/* This takes care when the user calls Qt.createComponent in javascript to load
 * the .qmc in place of creating a component from the .qml */
static QQmlComponent *loadCallback(QQmlEngine *engine, QUrl url, void *data)
{
    Q_UNUSED(engine);
    QmcLoader *loader = static_cast<QmcLoader*>(data);
    QString dotqml = ".qml";
    QString qmcfile = url.toString().replace("file://", "");
    qmcfile = qmcfile.replace(qmcfile.size() - dotqml.size(), dotqml.size(), ".qmc");
    qDebug() << "Loading Component" << qmcfile;
    return loader->loadComponent(qmcfile);
}

static bool jscLoadCallback(QV4::CallContext *ctx)
{
    if (!ctx->parent || !ctx->parent->compilationUnit || !ctx->parent->compilationUnit->data)
        return false; // Does not look like we can handle this.
    QmcUnit* includer = QmcUnit::findUnit(ctx->parent->compilationUnit->data);
    if (!includer) {
        qDebug() << "No corresponding QmcUnit found.";
        return false;
    }
    QmcScriptUnit *qsu = dynamic_cast<QmcScriptUnit*>(includer->blob);
    if (includer->loadedUrl.scheme() == "qrc") {
        qsu->setCacheBaseUrl(includer->url, includer->urlString);
    } else if (includer->loadedUrl.isLocalFile()) {
        qsu->setCacheBaseUrl(QUrl(includer->loadedUrl), includer->loadedUrl.toString());
    } else
        return false; // We do not load from network now.
    QString jscname = ctx->callData->args[0].toQStringNoThrow();
    jscname.append("c");
    QmcScriptUnit* script = includer->loader->getScript(jscname, includer->loadedUrl);
    if (!script)
        return false;
    script->initialize(includer);
    return true;
}


class QmcLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmcLoader)
public:
    QmcLoaderPrivate(QQmlEngine *engine);
    virtual ~QmcLoaderPrivate();
    QQmlEngine *engine;
    QList<QQmlError> errors;
    QmcTypeUnit* unit;
    QMap<QString, QmcUnit *> dependencies;
    bool loadDependenciesAutomatically;
    int dependencyRecursionDepth;
};

QmcLoaderPrivate::QmcLoaderPrivate(QQmlEngine *engine)
    : engine(engine),
      unit(NULL),
      loadDependenciesAutomatically(true),
      dependencyRecursionDepth(0)
{
    // Must add always since code compiled for debugging can be run without
    // -qmljsdebugger parameter.
    QV4::Object *go = QV8Engine::getV4(engine)->globalObject;
    QString checkName("checkBreakpoint");
    if (QV8Engine::getV4(engine)->debugger) {
        go->defineDefaultProperty(checkName, checkBreakpoint);
    } else {
        go->defineDefaultProperty(checkName, doNothing);
    }
    QV8Engine::getV4(engine)->setJscLoadCallback(jscLoadCallback);
}

QmcLoaderPrivate::~QmcLoaderPrivate()
{
    if (unit)
        unit->release();

    foreach (QmcUnit *unit, dependencies) {
        unit->blob->release();
    }
    dependencies.clear();
}

QmcLoader::QmcLoader(QQmlEngine *engine, QObject *parent) :
    QObject(*(new QmcLoaderPrivate(engine)), parent)
{
}

QQmlComponent *QmcLoader::loadComponent(const QString &file)
{
    Q_D(QmcLoader);
    if (QQmlFile::isBundle(file)) {
        // Untested at this point.
        QUrl url(file);
        QQmlBundleData *bundle = QQmlEnginePrivate::get(d->engine)->typeLoader.getBundle(url.host());
        if (!bundle) {
            QQmlError error;
            error.setDescription("Could not load bundle");
            error.setUrl(url);
            appendError(error);
            return NULL;
        }
        QString fileName = url.path().mid(1);
        const QQmlBundle::FileEntry *entry = bundle->find(fileName);
        if (!entry) {
            QQmlError error;
            error.setDescription("Could not find file in bundle");
            error.setUrl(QUrl(fileName));
            appendError(error);
            return NULL;
        }
        QByteArray contents(entry->contents(), entry->fileSize());
        QDataStream in(contents);
        QQmlComponent *ret = loadComponent(in, createLoadedUrl(file));
        bundle->release();
        return ret;
    }
    // This loads files and from bundles embedded in the application.
    QString fileName(file);
    if (file.startsWith("qrc:") || file.startsWith("file:")) {
        QUrl url(file);
        fileName = QQmlFile::urlToLocalFileOrQrc(url);
    }
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        QQmlError error;
        error.setDescription("Could not open file for reading");
        error.setUrl(QUrl(file));
        appendError(error);
        return NULL;
    }

    QDataStream in(&f);
    QQmlComponent *ret = loadComponent(in, createLoadedUrl(file));
    f.close();
    return ret;
}

QQmlComponent *QmcLoader::loadComponent(QDataStream &stream, const QUrl &loadedUrl)
{
    clearError();
    Q_D(QmcLoader);

    d->engine->setLoadCallback(loadCallback, this);

    // TBD: check validity of all read values
    QmcUnit *unit = QmcUnit::loadUnit(stream, d->engine, this, loadedUrl);
    if (!unit) {
        QQmlError error;
        error.setDescription("Error parsing / loading");
        error.setUrl(loadedUrl);
        appendError(error);
        return NULL;
    }

    // replace with loaded url cause we aren't always loading from the same
    // directories as we compiled
    unit->url = loadedUrl;
    unit->urlString = loadedUrl.toString();

    if (unit->type != QMC_QML) {
        QQmlError error;
        error.setDescription("Cannot have Script unit as main unit");
        error.setUrl(loadedUrl);
        appendError(error);
        unit->blob->release();
        return NULL;
    }

    QmcTypeUnit *typeUnit = (QmcTypeUnit *)unit->blob;

    // add to engine
    if (!typeUnit->link()) {
        appendErrors(unit->errors);
        unit->blob->release();
        return NULL;
    }

    // create QQmlComponent and attach QQmlCompiledData into it
    QQmlComponent *component = typeUnit->createComponent();
    if (!component) {
        QQmlError error;
        error.setDescription("Error creating QQmlComponent");
        error.setUrl(unit->url);
        appendError(error);
        unit->blob->release();
        return NULL;
    }

    // TBD: where to put unit
    d->unit = typeUnit;
    d->unit->addref();
    storeDebugData(stream);
    return component;
}

void QmcLoader::setLoadDependenciesAutomatically(bool load)
{
    Q_D(QmcLoader);
    d->loadDependenciesAutomatically = load;
}

bool QmcLoader::isLoadDependenciesAutomatically() const
{
    const Q_D(QmcLoader);
    return d->loadDependenciesAutomatically;
}

QUrl QmcLoader::createLoadedUrl(const QString &file)
{
    QString urlStr;
    if (file.startsWith("qrc:/"))
        urlStr = "";
    else if (file.startsWith(":/"))
        urlStr = "qrc";
    else if(file.startsWith("/"))
        urlStr = "file://";
    else{
        QDir dd;
        urlStr = "file://" + dd.absolutePath() + "/";
    }
    urlStr.append(file);
    return QUrl(urlStr);
}

QString QmcLoader::getBaseUrl(const QUrl &url)
{
    QString path = url.path();
    QUrl newUrl(url);
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash != -1) {
        path = path.left(lastSlash + 1);
        newUrl.setPath(path);
    } else {
        newUrl.setPath("");
    }
    return newUrl.toString();
}

QmcUnit *QmcLoader::getUnit(const QString &url)
{
    Q_D(QmcLoader);
    QString precompiled = precompiledUrl(url);
    if (!d->dependencies.contains(precompiled)) {
        if (d->loadDependenciesAutomatically) {
            // try to load it
            if (d->dependencyRecursionDepth >= DEPENDENCY_MAX_RECURSION_DEPTH) {
                QQmlError error;
                error.setDescription("Could not load dependency, too deep recursion");
                error.setUrl(url);
                appendError(error);
                return NULL;
            }
            d->dependencyRecursionDepth++;
            QmcUnit *unit = doloadDependency(precompiled);
            d->dependencyRecursionDepth--;
            return unit;
        } else
        return NULL;
    }
    return d->dependencies[precompiled];
}

QString QmcLoader::precompiledUrl(const QString &url)
{
    QString urlString = url;
    if (url.endsWith(".js"))
        urlString.append("c");
    else if (url.endsWith(".qml"))
        urlString[urlString.length() - 1] = 'c';
    return urlString;
}

QmcScriptUnit *QmcLoader::getScript(const QString &url, const QUrl &loaderUrl)
{
    QString newUrl;
    if(url.startsWith("file:")){
        newUrl = url;
    } else if (url.startsWith("qrc:")) {
        newUrl = url;
    }else{
        newUrl = getBaseUrl(loaderUrl);
        newUrl.append(url);
    }

    QmcUnit *unit = getUnit(newUrl);
    if (!unit)
        return NULL;
    if (unit->type != QMC_JS)
        return NULL;
    unit->blob->addref();
    return (QmcScriptUnit *)unit->blob;
}

QmcUnit *QmcLoader::getType(const QString &name, const QUrl &loaderUrl)
{
    qDebug() << "Getting type name = " << name << "loaderUrl =" << loaderUrl;

    QString newUrl;
    if(name.startsWith("file://")){
        newUrl = name;
        newUrl.append(".qml");
    }else{
        newUrl = getBaseUrl(loaderUrl);
        newUrl.append(name);
        newUrl.append(".qml");
    }

    QmcUnit *unit = getUnit(newUrl);
    if (!unit)
        return NULL;
    if (unit->type != QMC_QML)
        return NULL;
    unit->blob->addref();
    return unit;
}

bool QmcLoader::loadDependency(QDataStream &stream, const QUrl &loadedUrl)
{
    clearError();
    QmcUnit *unit = doloadDependency(stream, loadedUrl);
    unit->blob->release();
    return true;
}

bool QmcLoader::loadDependency(const QString &file)
{
    clearError();
    QUrl url(file);
    QString fileName = QQmlFile::urlToLocalFileOrQrc(url);
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        QQmlError error;
        error.setDescription("Could not open file for reading");
        error.setUrl(url);
        appendError(error);
        return NULL;
    }

    QDataStream in(&f);
    QmcUnit *unit = doloadDependency(in, createLoadedUrl(file));
    unit->blob->release();
    f.close();
    return true;
}

QmcUnit *QmcLoader::doloadDependency(const QString &url)
{
    QString file = QQmlFile::urlToLocalFileOrQrc(url);
    QFile f(file);
    if (!f.open(QFile::ReadOnly)) {
        QQmlError error;
        error.setDescription("Could not open file for reading: " + f.errorString());
        qWarning() << "Could not open file for reading" << file;
        error.setUrl(QUrl(file));
        appendError(error);
        return NULL;
    }

    QDataStream in(&f);
    const QUrl loadAs = createLoadedUrl(file);
    qDebug() << "Loading dependency" << url << "as" << loadAs;
    QmcUnit *unit = doloadDependency(in, loadAs);
    f.close();
    return unit;
}

QmcUnit *QmcLoader::doloadDependency(QDataStream &stream, const QUrl &loadedUrl)
{
    Q_D(QmcLoader);
    QmcUnit *unit = QmcUnit::loadUnit(stream, d->engine, this, loadedUrl);
    if (!unit) {
        QQmlError error;
        error.setDescription("Error loading");
        appendError(error);
        return NULL;
    }

    storeDebugData(stream);
    // add to dependencies
    d->dependencies[unit->loadedUrl.toString()] = unit;
    unit->blob->addref();
    return unit;
}

const QList<QQmlError>& QmcLoader::errors() const
{
    const Q_D(QmcLoader);
    return d->errors;
}

void QmcLoader::clearError() {
    Q_D(QmcLoader);
    d->errors.clear();
}

void QmcLoader::appendError(QQmlError error)
{
    Q_D(QmcLoader);
    d->errors.append(error);
}

void QmcLoader::appendErrors(const QList<QQmlError> &errors)
{
    Q_D(QmcLoader);
    d->errors.append(errors);
}

