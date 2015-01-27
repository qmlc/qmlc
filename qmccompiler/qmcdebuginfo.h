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

#ifndef QMCDEBUGINFO_H
#define QMCDEBUGINFO_H

#include "qmcfile.h"
#include "qmlcompilation.h"
#include "qmccompiler_global.h"

#include <QString>
#include <QByteArray>


class QMCCOMPILERSHARED_EXPORT QmcDebugInfo
{
public:
    QmcDebugInfo(const QString &name);
    void gatherInfo(const QmlCompilation &compilation);

    quint32 id() const { return info.sourceId; }

    bool append(const QString &fileName) const;

private:
    QmcDebug info;
};

#endif // QMCDEBUGINFO_H
