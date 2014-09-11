/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Copyright (C) 2014 Nomovok Ltd. All rights reserved.
** Contact: info@nomovok.com
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
//![0]

#include <QGuiApplication>
#include <QQmlComponent>
#include <QQuickView>
#include <QFile>
#include <QDebug>

#include "qmcloader.h"

int main(int argc, char *argv[])
{
    int ret;

    if(argc != 2 || !QFile::exists(argv[1])){
        qWarning() << argv[0] << "filename";
        return 1;
    }

    QGuiApplication app(argc, argv);

    QQuickView view;

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    //QQmlContext *ctxt = view.rootContext();

    QQmlEngine *engine = view.engine();

#if 1
    QmcLoader loader(engine);
    QQmlComponent *component = loader.loadComponent(argv[1]);
#else
    QQmlComponent *component = new QQmlComponent(engine, QUrl("qml/multipleitems.qml"));
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

    view.setContent(component->url(), component, rootObject);

    view.show();

    ret = app.exec();
    delete rootObject;
    delete component;
    return ret;

}
//![0]
