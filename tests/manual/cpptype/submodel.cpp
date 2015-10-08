/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Copyright (C) 2015 Nomovok Ltd. All rights reserved.
** Contact: info@nomovok.com
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

#include "submodel.h"
#include <QDebug>


SubModel::SubModel(QObject *parent)
    : QObject(parent), m_text("Original"), m_number(12345)
{
}

SubModel::SubModel(const SubModel &sub, QObject *parent)
    : QObject(parent), m_text(sub.text()), m_number(sub.number())
{
}

SubModel& SubModel::operator=(const SubModel &sub)
{
    if (this == &sub)
        return *this;
    m_text = sub.text();
    m_number = sub.number();
    return *this;
}

SubModel& SubModel::operator=(const SubModel *sub)
{
    return *this = *sub;
}

QString SubModel::text() const
{
    return m_text;
}

void SubModel::setText(QString value)
{
    bool changed = value != m_text;
    m_text = value;
    if (changed)
        emit textChanged(m_text);
}

int SubModel::number() const
{
    return m_number;
}

void SubModel::setNumber(int value)
{
    bool changed = m_number != value;
    m_number = value;
    if (changed)
        emit numberChanged();
}
