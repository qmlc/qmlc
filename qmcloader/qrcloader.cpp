
#include <QDirIterator>
#include <QXmlStreamReader>

#include <qqmlengine.h>
#include <qqmlcontext.h>
#include <qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include <private/qv4engine_p.h>
#include <private/qqmlengine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qv4isel_masm_p.h>

#include "qrcloader.h"

QrcLoader::QrcLoader(QQmlEngine *engine):
    loader(new QmcLoader(engine))
{
}

QrcLoader::~QrcLoader(void)
{
    delete loader;
}

int QrcLoader::load(QString topLevelQmc, QString qrcFile)
{
    int ret;

    ret = parseQrc(qrcFile);
    if(ret != 0){
        return ret;
    }

    ret = loadDependencies(topLevelQmc);
    if(ret != 0){
        return ret;
    }

    ret = loadTopLevelQmc(topLevelQmc);
    return ret;
}

QQmlComponent *QrcLoader::getRootComponent(void)
{
    return rootComponent;
}

/* get all the filenames we are intested in from qrcFile */
int QrcLoader::parseQrc(QString qrcFile)
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
                qDebug() << filename;
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


int QrcLoader::loadDependencies(QString topLevelQmc)
{
    int ret;
    QString fullPath;

    QString topLevelQml = topLevelQmc;
    topLevelQml.replace(".qmc", ".qml");

    // iterate through files in .qrc and compile the ones we support
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QDir dir = it.next();

        // skip top level .qmc
        //if(dir.path().contains(topLevelQml)){
        if(topLevelQml.contains(dir.path())){
            continue;
        }

        ret = 0;
        QString file;
        if(dir.path().endsWith(".qml")){
            foreach(file, qrcQmlFiles){
                if(dir.path().contains(file)){
                    fullPath = dir.path();
                    fullPath.replace(".qml", ".qmc");
                    file.replace(".qml", ".qmc");
                    break;
                }
            }
        }else if(dir.path().endsWith(".js")){
            foreach(file, qrcJsFiles){
                if(dir.path().contains(file)){
                    fullPath = dir.path();
                    fullPath.replace(".js", ".jsc");
                    file.replace(".js", ".jsc");
                    break;
                }
            }
        }

        if(file.count() > 0){
            //QString path = dir.path().mid(2);
            QFile inputFile(file);
            if (!inputFile.open(QFile::ReadOnly)){
                qWarning() << "Couldn't open dependency for input" << file;
                return -1;
            }

            QDataStream in(&inputFile);

            QString qrcPath = "qrc" + fullPath;
            qDebug() << "Loading dependency " << qrcPath;
            bool success = loader->loadDependency(in, QUrl(qrcPath));
            qDebug() << (success ? "OK" : "Failed");
            assert(success);
            inputFile.close();
            assert(success);

            if(ret){
                return ret;
            }
        }
    }
    return 0;
}

int QrcLoader::loadTopLevelQmc(QString qmcFile)
{

    QString qmlFile = qmcFile;
    qmlFile.replace(".qmc", ".qml");

    QString file;
    qDebug() << qmlFile;
    foreach(file, qrcQmlFiles){
        qDebug() << qmlFile << file;
        if(qmlFile.contains(file)){
            file.replace(".qml", ".qmc");
            break;
        }
    }

    if(file.count() == 0){
        return -1;
    }

    QFile inputFile(file);
    if (!inputFile.open(QFile::ReadOnly)){
        qWarning() << "Couldn't open file for input" << file;
        return -2;
    }

    QDataStream in(&inputFile);
    QString qrcPath = "qrc" + qmcFile;

    qDebug() << "Loading component " << qrcPath;
    rootComponent = loader->loadComponent(in, QUrl(qrcPath));
    qDebug() << (rootComponent ? "OK" : "Failed");

    inputFile.close();

    if(!rootComponent){
        return -3;
    }

    return 0;
}
