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

#include "qmcdebuginfo.h"

#include <QHash>
#include <QtDebug>


QmcDebugInfo::QmcDebugInfo(const QString &name)
{
    info.sourceId = 0;
    info.sourceName = name;
}

void QmcDebugInfo::gatherInfo(const QmlCompilation &compilation)
{
    info.sourceId = qHash(compilation.code);
}

bool QmcDebugInfo::append(const QString &fileName) const
{
    QFile f(fileName);
    if (!f.open(QIODevice::Append)) {
        qWarning() << "Error: could not open output file for append.";
        return false;
    }
    QDataStream out(&f);
    out << QMC_DEBUG_INFO_FOOTER;
    out << info.sourceId;
    out << info.sourceName;
    if (out.status() != QDataStream::Ok) {
        qWarning() << "Error: could not write debug info to output file.";
        return false;
    }
    f.close();
    return true;
}

