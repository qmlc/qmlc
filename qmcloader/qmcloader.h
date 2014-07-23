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

#ifndef QMCLOADER_H
#define QMCLOADER_H

#include "qmcloader_global.h"

#include <QObject>
#include <QQmlError>

class QQmlEngine;
class QDataStream;
class QQmlComponent;

class QmcLoaderPrivate;
class QmcUnit;
class QmcScriptUnit;


class QMCLOADERSHARED_EXPORT QmcLoader : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QmcLoader)
public:
    explicit QmcLoader(QQmlEngine *engine, QObject *parent = 0);
    QQmlComponent *loadComponent(QDataStream &stream, const QUrl &loadedUrl);
    QQmlComponent *loadComponent(const QString &file);
    bool loadDependency(QDataStream &stream, const QUrl &loadedUrl);
    bool loadDependency(const QString &file);
    const QList<QQmlError>& errors() const;
    QmcScriptUnit *getScript(const QString &url, const QUrl &loaderUrl);
    QmcUnit *getType(const QString &name, const QUrl &loaderUrl);
    void setLoadDependenciesAutomatically(bool load);
    bool isLoadDependenciesAutomatically() const;

private:
    QUrl createLoadedUrl(const QString &url);
    QmcUnit *doloadDependency(const QString &url);
    QmcUnit *doloadDependency(QDataStream &stream, const QUrl &loadedUrl);
    void appendError(QQmlError error);
    void appendErrors(const QList<QQmlError>& errors);
    void clearError();
    QString getBaseUrl(const QUrl &url);
    QmcUnit *getUnit(const QString &url);
};

#endif // QMCLOADER_H
