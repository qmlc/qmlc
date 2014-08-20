
#include <QCoreApplication>
#include <QDirIterator>
#include <QXmlStreamReader>

#include <qqmlengine.h>

#include <private/qv4engine_p.h>
#include <private/qqmlengine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qv4isel_masm_p.h>

#include "qrccompiler.h"

#include "qmlc.h"
#include "scriptc.h"

int QrcCompiler::compile(int argc, char **argv, const QString &projectBaseDir,
        const QString &qrcFile)
{
    int ret;

    QCoreApplication app(argc, argv);

    engine = new QQmlEngine;

    // round about way to add projectBaseDir to path because
    // running a QCoreApplication and addImportPath doesn't work
    QByteArray array = projectBaseDir.toLocal8Bit();
    char *baseDir = array.data();
    setenv("QML2_IMPORT_PATH", baseDir, 1);

    ret = parseQrc(qrcFile);
    if(ret != 0){
        return ret;
    }

    // iterate through files in .qrc and compile the ones we support
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {

        QDir dir = it.next();
        //qDebug() << dir.path();

        ret = 0;
        if(dir.path().endsWith(".qml")){
            foreach(QString file, qrcQmlFiles){
                if(dir.path().contains(file)){
                    qDebug() << "Compiling" << file;
                    ret = compileQml("qrc" + dir.path(), projectBaseDir + "/" +
                            file.replace(".qml", ".qmc"));
                    break;
                }
            }
        }else if(dir.path().endsWith(".js")){
            foreach(QString file, qrcJsFiles){
                if(dir.path().contains(file)){
                    qDebug() << "Compiling" << file;
                    ret = compileJs("qrc" + dir.path(), projectBaseDir + "/" +
                            file.replace(".js", ".jsc"));
                    break;
                }
            }
        }

        if(ret){
            qDebug() << "Failed";
            return ret;
        }
    }

    qDebug("Compilation done");

    delete engine;

    return ret;
}

/* get all the filenames we are intested in from qrcFile */
int QrcCompiler::parseQrc(QString qrcFile)
{
    QFile qrc(qrcFile);

    if(!qrc.open(QIODevice::ReadOnly)){
        qDebug() << "File open error:" << qrc.errorString();
        return qrc.error();
    }

    QXmlStreamReader inputStream(&qrc);

    while (!inputStream.atEnd() && !inputStream.hasError()) {
        inputStream.readNext();
        if (inputStream.isStartElement()) {
            QString name = inputStream.name().toString();
            if (name == "file"){
                QString filename = inputStream.readElementText();
                //qDebug() << filename;
                if(filename.endsWith(".qml")){
                    qrcQmlFiles.append(filename);
                }else if(filename.endsWith(".js")){
                    qrcJsFiles.append(filename);
                }
            }
        }
    }

    qrc.close();
    return 0;
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

