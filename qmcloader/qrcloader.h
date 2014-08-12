#ifndef QRCLOADER_H
#define QRCLOADER_H

#include "qmcloader.h"

class QrcLoader
{

public:
    QrcLoader(QQmlEngine *engine);
    ~QrcLoader(void);

    int load(QString topLevelQmc);
    QQmlComponent *getRootComponent(void);

private:
    QQmlComponent *rootComponent;
    QmcLoader *loader;

    int loadDependencies(QString topLevelQmc);
    int loadTopLevelQmc(QString qmcFile);

};

#endif
