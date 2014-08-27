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

#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QTimer>
#include <QQmlEngine>

#include <iostream>
#include "qmlc.h"
#include "scriptc.h"
#include "comp.h"

using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QQmlEngine *engine = new QQmlEngine;
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " input-file" << endl;
        return EXIT_FAILURE;
    }
    QString fileName(argv[1]);

    if (fileName.lastIndexOf('.') <= 0) {
        cerr << "Filename cannot be empty";
        return EXIT_FAILURE;
    }

    // update import path to include .
    // engine->addImportPath(".");  doesn't work?
    char *curvalue = getenv("QML2_IMPORT_PATH");
    if(curvalue){
        char *newvalue = (char *)malloc(strlen(curvalue) + 2);
        if(!newvalue){
            qWarning("malloc failed, couldn't update QML_IMPORT_PATH");
        }else{
            sprintf(newvalue, "%s:%s", curvalue, ".");
            setenv("QML2_IMPORT_PATH", newvalue, 1);
            free(newvalue);
        }
    }else{
        setenv("QML2_IMPORT_PATH", ".", 1);
    }

    Compiler *compiler = NULL;
    if (fileName.endsWith(".js")) {
        compiler = new ScriptC(engine);
    } else if (fileName.endsWith(".qml")) {
        compiler = new QmlC(engine);
    } else {
        cerr << "Supported filetypes include .js and .qml" << endl;
        return EXIT_FAILURE;
    }
    Comp comp;
    comp.compiler = compiler;
    comp.fileName = fileName;

    /*
    QObject::connect(&comp, SIGNAL(finished()), &app, SLOT(quit()));
    QTimer::singleShot(0, &comp, SLOT(compile()));

    app.exec();
    */
    comp.compile();

    delete compiler;
    delete engine;
    if (Comp::retValue == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
