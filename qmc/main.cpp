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

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QDataStream>
#include <QTimer>
#include <QQmlEngine>

#include <iostream>
#include "qmlc.h"
#include "scriptc.h"
#include "comp.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QQmlEngine *engine = new QQmlEngine;
    if (argc != 2) {
        qWarning() << "Usage:" << argv[0] << " input-file";
        return EXIT_FAILURE;
    }
    QString fileName(argv[1]);

    if (fileName.lastIndexOf('.') <= 0) {
        qWarning() << "Error: Filename cannot be empty";
        return EXIT_FAILURE;
    }

    if (!QFile::exists(fileName)) {
        qWarning() << "Error:" << fileName << "doesn't exist";
        return EXIT_FAILURE;
    }

    // update import path to include .
    // engine->addImportPath(".");  doesn't work?
    char *curvalue = getenv("QML2_IMPORT_PATH");
    if(curvalue){
        char *newvalue = (char *)malloc(strlen(curvalue) + 2);
        if(!newvalue){
            qWarning() << "Error: malloc failed, couldn't update QML_IMPORT_PATH";
            return EXIT_FAILURE;
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
        qWarning() << "Error: Supported filetypes include .js and .qml";
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
    else{
        return EXIT_FAILURE;
    }
}
