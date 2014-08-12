
#include <QCoreApplication>
#include <QDirIterator>

#include <qqmlengine.h>

#include <private/qv4engine_p.h>
#include <private/qqmlengine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qv4isel_masm_p.h>

#include "qrccompiler.h"

#include "qmlc.h"
#include "scriptc.h"

int QrcCompiler::compile(int argc, char **argv, const QString &projectBaseDir)
{
    int ret;

    QCoreApplication app(argc, argv);

    engine = new QQmlEngine;

    // iterate through files in .qrc and compile the ones we support
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {

        QDir dir = it.next();
        //qDebug() << dir.path();

        ret = 0;
        if(dir.path().endsWith(".qml")){
            ret = compileQml("qrc" + dir.path(),
                    projectBaseDir + dir.dirName().replace(".qml", ".qmc"));
        }else if(dir.path().endsWith(".js")){
            ret = compileJs("qrc" + dir.path(),
                    projectBaseDir + dir.dirName().replace(".js", ".jsc"));
        }

        if(ret){
            return ret;
        }
    }

    delete engine;

    return ret;
}

int QrcCompiler::compileQml(const QString &inputFile, const QString &outputFile)
{
    QmlC c(engine);

    QFile f(outputFile);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Couldn't open file for output" << outputFile;
        return -1;
    }
    QDataStream output(&f);

    bool success = c.compile(inputFile, output);

    f.close();

    if (!success){
        qDebug() << "Compilation failed" << inputFile;
        f.remove();
        return -2;
    }

    return 0;

}

int QrcCompiler::compileJs(const QString &inputFile, const QString &outputFile)
{
    ScriptC jsc(engine);

    QFile f(outputFile);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Couldn't open file for output" << outputFile;
        return -3;
    }
    QDataStream output(&f);

    bool success = jsc.compile(inputFile, output);

    f.close();

    if (!success){
        qWarning() << "JS Compilation failed" << inputFile;
        f.remove();
        return -4;
    }

    return 0;
}
