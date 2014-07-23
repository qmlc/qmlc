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

#ifndef TESTCREATEFILE_H
#define TESTCREATEFILE_H

#include <QObject>
#include <QTemporaryDir>

class TestCreateFile : public QObject
{
    Q_OBJECT
public:
    explicit TestCreateFile(QObject *parent = 0);
    virtual ~TestCreateFile();

private slots:
    void initTestCase();
    void cleanupTestCase();

private:
    QTemporaryDir* tempDir;
};

#endif // TESTCREATEFILE_H
