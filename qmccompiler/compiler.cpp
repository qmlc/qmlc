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
};

CompilerPrivate::CompilerPrivate()
{
}

CompilerPrivate::~CompilerPrivate()
{
}

Compiler::Compiler(QObject *parent) :
    QObject(*(new CompilerPrivate), parent)
{
}

Compiler::~Compiler()
{
}

bool Compiler::loadData(const QUrl &url, QmlCompilation *compilation)
{
    if (!url.isValid() || url.isEmpty())
        return false;
    QQmlFile f;
    f.load(compilation->engine, url);
    if (!f.isReady()) {
        if (f.isError()) {
            QQmlError error;
            error.setUrl(url);
            error.setDescription(f.error());
            appendError(error);
        }
        return false;
    }
    compilation->code = QString::fromUtf8(f.data());
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

bool Compiler::compile(const QString &url, QDataStream &output)
{
    clearError();

    // check that engine is using correct factory
    if (!qgetenv("QV4_FORCE_INTERPRETER").isEmpty()) {
        QQmlError error;
        error.setDescription("Compiler is forced to use interpreter");
        appendError(error);
        return false;
    }

    QmlCompilation* c = new QmlCompilation(url, QUrl(url));

    if (!loadData(url, c)) {
        delete c;
        return false;
    }

    if (!compileData(c)) {
        delete c;
        return false;
    }

    if (!c->checkData()) {
        delete c;
        QQmlError error;
        error.setDescription("Compiled data not valid. Internal error.");
        appendError(error);
        return false;
    }

    bool ret = exportData(c, output);
    delete c;
    return ret;
}

bool Compiler::exportData(QmlCompilation *compilation, QDataStream &output)
{
    QmcExporter exporter(compilation);
    bool ret = exporter.exportQmc(output);
    if (!ret) {
        QQmlError error;
        error.setDescription("Error saving data");
        appendError(error);
    }
    return ret;
}
