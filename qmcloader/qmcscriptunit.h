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

#ifndef QMCSCRIPTUNIT_H
#define QMCSCRIPTUNIT_H

#include <QList>

#include <private/qqmltypeloader_p.h>

class QmcUnit;

class QmcScriptUnit : public QQmlScriptBlob
{
public:
    QmcScriptUnit(QmcUnit *unit, QQmlTypeLoader *typeLoader);
    virtual ~QmcScriptUnit();

    bool initialize();

private:
    QmcUnit *unit;
    QList<QmcUnit *> dependencies;
};

#endif // QMCSCRIPTUNIT_H
