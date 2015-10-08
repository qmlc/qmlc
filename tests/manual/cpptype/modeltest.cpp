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

#include "modeltest.h"
#include <QDebug>


ModelTest::ModelTest(QObject *parent)
    : QObject(parent), m_strprop("Original"), m_intprop(12345)
{
    m_sub = new SubModel(this);
}

ModelTest::~ModelTest()
{
    delete m_sub;
}

QString ModelTest::strprop() const
{
    return m_strprop;
}

void ModelTest::setStrprop(QString value)
{
    bool changed = value != m_strprop;
    m_strprop = value;
    if (changed)
        emit strpropChanged(m_strprop);
}

void ModelTest::changeProperty(QString value)
{
    setStrprop(value);
}

int ModelTest::intprop() const
{
    return m_intprop;
}

void ModelTest::setIntprop(int value)
{
    bool changed = m_intprop != value;
    m_intprop = value;
    if (changed)
        emit intpropChanged();
}

SubModel *ModelTest::sub() const
{
    return m_sub;
}

void ModelTest::setSub(const SubModel *value)
{
    delete m_sub;
    m_sub = new SubModel(*value, this);
    emit subChanged();
}

