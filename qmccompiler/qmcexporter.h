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

#ifndef QMCEXPORTER_H
#define QMCEXPORTER_H

#include <QObject>
#include <QDataStream>

#include "qmcfile.h"
#include "qmccompiler_global.h"

class QmlCompilation;

class QMCCOMPILERSHARED_EXPORT QmcExporter : public QObject
{
    Q_OBJECT
public:
    QmcExporter(QmlCompilation *compilation, QObject* parent = NULL);

    bool exportQmc(QDataStream &stream);

private:
    void createHeader(QmcUnitHeader &header, QmlCompilation *c);
    bool writeQmcUnit(QmlCompilation *c, QDataStream &stream);
    bool writeString(QDataStream& stream, QString string);
    bool writeData(QDataStream& stream, const char *data, int len);
    bool writeDataWithLen(QDataStream& stream, const char* data, int len);
    bool writeBitArray(QDataStream& stream, const QBitArray& array);
    QmlCompilation *compilation;

};

#endif // QMCEXPORTER_H
