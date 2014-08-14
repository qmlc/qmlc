
#include <QDirIterator>

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

int QrcLoader::load(QString topLevelQmc)
{
    int ret;

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

int QrcLoader::loadDependencies(QString topLevelQmc)
{
    int ret;

    // iterate through files in .qrc and compile the ones we support
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QDir dir = it.next();

        // skip top level .qmc
        if(QString(":/" + topLevelQmc) == dir.path()){
            continue;
        }

        ret = 0;
        if(dir.path().endsWith(".qmc") || dir.path().endsWith(".jsc")){
            //ret = compileFile(engine, "qrc" + dir.path(),
             //       outputBaseDir + dir.dirName().replace(".qml", ".qmc"));
            // remove leading :/
            QString path = dir.path().mid(2);
            QFile inputFile(path);
            if (!inputFile.open(QFile::ReadOnly)){
                qWarning() << "Couldn't open file for input" << path;
                return -1;
            }

            QDataStream in(&inputFile);

            qDebug() << "Loading.." << "qrc" + dir.path();
            bool success = loader->loadDependency(in, QUrl("qrc" + dir.path()));
            assert(success);
            inputFile.close();
            assert(success);
        }

        if(ret){
            return ret;
        }
    }
    return 0;
}

int QrcLoader::loadTopLevelQmc(QString qmcFile)
{

    QFile inputFile(qmcFile);
    if (!inputFile.open(QFile::ReadOnly)){
        qWarning() << "Couldn't open file for input" << qmcFile;
        return -2;
    }

    QDataStream in(&inputFile);

    rootComponent = loader->loadComponent(in, QUrl("qrc:/" + qmcFile));

    inputFile.close();

    if(!rootComponent){
        return -3;
    }

    return 0;
}

