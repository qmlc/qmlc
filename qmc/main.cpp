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
#include <QGuiApplication>
#include <private/qv4context_p.h>
#include <private/qv8engine_p.h>
#include <QLibrary>

#include <iostream>
#include "qmlc.h"
#include "scriptc.h"
#include "comp.h"
#include "qmcdebuginfo.h"

typedef bool (*TypeRegisterer)();

QV4::ReturnedValue checkBreakpoint(QV4::CallContext *ctx)
{
    return QV4::Encode::undefined();
}

int main(int argc, char *argv[])
{
    int i;
    bool bCoreApp = true;

    for (i=0;i<argc;i++) {
        if (strcmp(argv[i], "-u") == 0) {
            bCoreApp = false;
        }
    }

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
    QCommandLineOption nameOption("n",
        QCoreApplication::translate("main", "File name for debug info."),
        QCoreApplication::translate("main", "source name"));
    parser.addOption(nameOption);
    QCommandLineOption guiOption("u",
        QCoreApplication::translate("main", "Use QGuiApplication."));
    parser.addOption(guiOption);
    QCommandLineOption typelibOption("t",
        QCoreApplication::translate("main", "Type library to load."),
        QCoreApplication::translate("main", "libraryfile[,...]"));
    parser.addOption(typelibOption);

#if defined(DEBUG_QMC)
    QCommandLineOption debugOutput("debug",
        QCoreApplication::translate("main", "Compiler debug output."));
    parser.addOption(debugOutput);
#endif

    QCoreApplication *app = NULL;
    if (bCoreApp == true) {
        app = new QCoreApplication(argc, argv);
    } else {
        app = new QGuiApplication(argc, argv);
    }
    parser.process(app->arguments());

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

    // Load type libraries before current path changes.
    if (parser.isSet(typelibOption)) {
        bool error = false;
        QStringList allLibs = parser.value(typelibOption).split(",", QString::SkipEmptyParts);
        foreach (const QString &name, allLibs) {
            QLibrary *lib = new QLibrary(name, app);
            lib->setLoadHints(QLibrary::ResolveAllSymbolsHint);
            if (!lib->load()) {
                qWarning() << lib->errorString();
                error = true;
                continue;
            }
            TypeRegisterer func = (TypeRegisterer)lib->resolve("registerQmlTypes");
            if (!func) {
                qWarning() << lib->errorString();
                error = true;
                continue;
            }
            if (!func()) {
                qWarning() << "Register problem in:" << name;
                error = true;
            }
        }
        if (error)
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
    QString debugName = fileName;
    if (parser.isSet(nameOption)) {
        debugName = parser.value(nameOption);
    }
    QFileInfo iname(fileName);
    QDir dir = iname.dir();
    QDir::setCurrent(dir.path());
    fileName = iname.fileName();
    // A side effect is that compiler error messages lose path in file name.
    // Add original file name to Comp if this is a problem.

    CompilerOptions options;
    if (parser.isSet(debugOption)) {
        options.debug = new QmcDebugInfo(debugName);
    }

#if defined(DEBUG_QMC)
    if (parser.isSet(debugOutput)) {
        options.debugOutput = true;
    }
#endif

    QQmlEngine *engine = new QQmlEngine;
    // Technically might not be needed but added just to be on the safe side.
    if (options.debug) {
        QV4::Object *go = QV8Engine::getV4(engine)->globalObject;
        go->defineDefaultProperty(QStringLiteral("checkBreakpoint"), checkBreakpoint);
    }

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
        compiler = new ScriptC(engine, &options);
    } else if (fileName.endsWith(".qml")) {
        compiler = new QmlC(engine, &options);
    } else {
        qWarning() << "Error: Supported filetypes include .js and .qml";
        return EXIT_FAILURE;
    }
    Comp comp;
    comp.compiler = compiler;
    comp.fileName = fileName;
    comp.outputFileName = outputFileName;

    comp.compile();

    if (options.debug && !options.debug->append(comp.outputFileName)) {
        qWarning() << "Appending debug info failed.";
        return EXIT_FAILURE; // Technically file would execute properly.
    }

    delete compiler;
    delete engine;
    if (Comp::retValue != 0)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
