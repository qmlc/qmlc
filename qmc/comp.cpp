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

#include <QFile>
#include <QDataStream>

#include "comp.h"
#include "compiler.h"

#include <iostream>

using std::cerr;
using std::endl;

Comp::Comp(QObject *parent) :
    QObject(parent)
{
}

Comp::~Comp()
{
}

int Comp::retValue = EXIT_FAILURE;

void Comp::compile()
{
    if (outputFileName.isEmpty()) {
        outputFileName = fileName;
        if (outputFileName.endsWith("qml"))
            outputFileName[outputFileName.size() - 1] = 'c';
        else
            outputFileName.append("c");
    }

    bool ret = compiler->compile("file:" + fileName, outputFileName);

    if (ret){
        retValue = EXIT_SUCCESS;
    } else {
        if (compiler->errors().empty())
            cerr << "Error compiling <no reason>" << endl;
        foreach (QQmlError error, compiler->errors()) {
            cerr << "Error: " << error.toString().toStdString() << endl;
        }
    }
    emit finished();
    return;
}
