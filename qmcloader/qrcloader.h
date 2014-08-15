#ifndef QRCLOADER_H
#define QRCLOADER_H

#include "qmcloader.h"

class QrcLoader
{

public:
    QrcLoader(QQmlEngine *engine);
    ~QrcLoader(void);

    int load(QString topLevelQmc, QString qrcFile);
    QQmlComponent *getRootComponent(void);

private:
    QStringList qrcQmlFiles;
    QStringList qrcJsFiles;
    QQmlComponent *rootComponent;
    QmcLoader *loader;

    int parseQrc(QString qrcFile);
    int loadDependencies(QString topLevelQmc);
    int loadTopLevelQmc(QString qmcFile);

};

#endif
