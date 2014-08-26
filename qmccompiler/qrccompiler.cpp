
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

QrcCompiler::QrcCompiler():
    exitOnError(true)
{
}

void QrcCompiler::exitOnCompileError(bool exit)
{
    exitOnError = exit;
}

int QrcCompiler::compile(int argc, char **argv, const QString &projectBaseDir,
        const QString &qrcFile)
{
    int ret;

    QCoreApplication app(argc, argv);

    engine = new QQmlEngine;

    QDir::setCurrent(projectBaseDir);
    QString absProjectBaseDir = QDir::currentPath();
    qDebug() << "absProjectBaseDir" << absProjectBaseDir;
    setenv("QML2_IMPORT_PATH", ".", 1);

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
                    QFileInfo fileInfo(file);
                    QString in = fileInfo.fileName();
                    QString out = in;
                    out.replace(".qml", ".qmc");
                    QDir::setCurrent(fileInfo.path());
                    ret = compileQml("file:" + in, out);
                    QDir::setCurrent(absProjectBaseDir);
                    break;
                }
            }
        }else if(dir.path().endsWith(".js")){
            foreach(QString file, qrcJsFiles){
                if(dir.path().contains(file)){
                    qDebug() << "Compiling" << file;
                    QFileInfo fileInfo(file);
                    QString in = fileInfo.fileName();
                    QString out = in;
                    out.replace(".js", ".jsc");
                    QDir::setCurrent(fileInfo.path());
                    ret = compileJs("file:" + in, out);
                    QDir::setCurrent(absProjectBaseDir);
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
        qWarning() << "Qml Compilation failed" << inputFile;
        if (c.errors().empty()){
            qWarning() << "Error compiling <no reason>";
        }else{
            foreach (QQmlError error, c.errors()) {
                qWarning() << "Error: " << error.toString();
            }
        }

        f.remove();
        if(exitOnError){
            return -2;
        }
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
        if (jsc.errors().empty()){
            qWarning() << "Error compiling <no reason>";
        }else{
            foreach (QQmlError error, jsc.errors()) {
                qWarning() << "Error: " << error.toString();
            }
        }
        f.remove();
        if(exitOnError){
            return -4;
        }
    }

    return 0;
}

