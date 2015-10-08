/* Copyright (C) 2014 Timo Hannukkala <timo.hannukkala@nomovok.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QGuiApplication>
#ifdef HAS_BOOSTER
#include <mdeclarativecache5/MDeclarativeCache>
#endif
#include <QQmlComponent>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDir>
#include <QDebug>

#include "qmcloader/qmcloader.h"
#include "testlistmodel.h"
#include "testlistmodel2.h"



int main(int argc, char *argv[])
{
  int ret;

    QGuiApplication app(argc, argv);

    QQuickView *pParentView = new QQuickView();

    pParentView->setResizeMode(QQuickView::SizeRootObjectToView);

    QQmlEngine *engine = pParentView->engine();
    qmlRegisterType<TestListModel2>("TestListModel2",1,0,"TestListModel2");


#if 1
    QDir::setCurrent(QGuiApplication::applicationDirPath());
    QmcLoader loader(engine);
    QQmlComponent *component = loader.loadComponent("../qml/main.qmc");
#else
    QQmlComponent *component = new QQmlComponent(engine, QUrl("../qml/main.qmc"));
#endif

    if (!component) {
        qDebug() << "Could not load component";
        return -1;
    }

    if (!component->isReady()) {
        qDebug() << "Component is not ready";
        if (component->isError()) {
            foreach (const QQmlError &error, component->errors()) {
                qDebug() << error.toString();
            }
        }
        return -1;
    }

    QObject *rootObject = component->create();
    if (!rootObject) {
        qDebug() << "Could not create root object";
        return -1;
    }

    QObject::connect(pParentView->engine(), SIGNAL(quit()), &app, SLOT(quit()));
    pParentView->setContent(component->url(), component, rootObject);


    pParentView->requestActivate();    	
    pParentView->raise();
    pParentView->showFullScreen();

    ret = app.exec();
    delete rootObject;
    delete component;
    return ret;

}
