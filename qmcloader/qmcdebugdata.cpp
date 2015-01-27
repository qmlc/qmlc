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
 * Author: Ismo Karkkainen <ismo.karkkainen@nomovok.com>
 */

#include "qmcdebugdata.h"
#include <QHash>
#include <QtDebug>


static QHash<quint32,QmcDebug*> map;


void storeDebugData(QDataStream &input)
{
    if (input.atEnd())
        return; // No debug data present.
    quint32 footer;
    input >> footer;
    QmcDebug *debug = new QmcDebug();
    input >> debug->sourceId;
    input >> debug->sourceName;
    if (input.status() == QDataStream::ReadCorruptData ||
        input.status() == QDataStream::ReadPastEnd ||
        footer != QMC_DEBUG_INFO_FOOTER)
    {
        qDebug() << "Debug data read failure: " << footer << debug->sourceId << debug->sourceName;
        delete debug;
        return; // Ignore failures to read data.
    }
    if (map.contains(debug->sourceId)) {
        QmcDebug *old = debugData(debug->sourceId);
        qWarning() << "Debug sourceId collision: " << old->sourceId << " " <<
            old->sourceName << " & " << debug->sourceName;
    }
    map.insert(debug->sourceId, debug);
}

QmcDebug *debugData(quint32 key)
{
    QHash<quint32,QmcDebug*>::iterator iter = map.find(key);
    if (iter != map.end())
        return iter.value();
#if defined(DEBUG)
    static quint32 last_key = 0;
    if (key == last_key) return NULL;
    foreach (quint32 k, map.keys()) {
        QmcDebug *val = map.find(k).value();
        qDebug() << "Debug data map: " << k << val->sourceId << val->sourceName;
    }
    last_key = key;
#endif
    return NULL;
}

