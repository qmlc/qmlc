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
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDir>

#include <iostream>
#include "qmlc.h"
#include "scriptc.h"
#include "comp.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("QML/JS compiler.");
    parser.addHelpOption();
    parser.addPositionalArgument("source",
        QCoreApplication::translate("main", "Source file."));
    QCommandLineOption debugOption("g",
        QCoreApplication::translate("main", "Add debug information."));
    parser.addOption(debugOption);
    QCommandLineOption outputOption("o",
        QCoreApplication::translate("main", "Speficy output file."),
        QCoreApplication::translate("main", "file name"));
    parser.addOption(outputOption);

    parser.process(app);
    bool debug = parser.isSet(debugOption);

    // Input and output file names.
    const QStringList inputNames = parser.positionalArguments();
    if (inputNames.size() != 1) {
        qWarning() << "Input source file name must be given.";
        return EXIT_FAILURE;
    }
    QString fileName(inputNames[0]);
    if (fileName.lastIndexOf('.') <= 0) {
        qWarning() << "Error: Filename cannot be empty";
        return EXIT_FAILURE;
    }
    if (!QFile::exists(fileName)) {
        qWarning() << "Error:" << fileName << "doesn't exist";
        return EXIT_FAILURE;
    }
    // Compilation sometimes need to happen in the same directory as the source
    // file. Therefore change to source directory and adjust the output file
    // path accordingly, if it has been given.
    QString outputFileName;
    if (parser.isSet(outputOption)) {
        QFileInfo oname(parser.value(outputOption));
        outputFileName = oname.absoluteFilePath();
    }
    QFileInfo iname(fileName);
    QDir dir = iname.dir();
    QDir::setCurrent(dir.path());
    fileName = iname.fileName();
    // A side effect is that compiler error messages lose path in file name.
    // Add original file name to Comp if this is a problem.

    QQmlEngine *engine = new QQmlEngine;

    // update import path to include .
    // engine->addImportPath(".");  doesn't work?
    char *curvalue = getenv("QML2_IMPORT_PATH");
    if(curvalue){
        char *newvalue = (char *)malloc(strlen(curvalue) + 3);
        if(!newvalue){
            qWarning() << "Error: malloc failed, couldn't update QML_IMPORT_PATH";
            return EXIT_FAILURE;
        }
        sprintf(newvalue, "%s:%s", curvalue, ".");
        setenv("QML2_IMPORT_PATH", newvalue, 1);
        free(newvalue);
    }else{
        setenv("QML2_IMPORT_PATH", ".", 1);
    }

    // TODO: pass debug flag presence to compilers.
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
    comp.outputFileName = outputFileName;

    comp.compile();

    delete compiler;
    delete engine;
    if (Comp::retValue != 0)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
