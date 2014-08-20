#include "cppsubitem.h"
#include <QObject>

CppSubItem::CppSubItem(QObject *parent)
    : QObject(parent)
{
    m_name = "default_name";
    m_color = "red";
}

QString CppSubItem::name() const
{
    return m_name;
}

void CppSubItem::setName(const QString &name)
{
    m_name = name;
}

QColor CppSubItem::color() const
{
    return m_color;
}

void CppSubItem::setColor(const QColor &color)
{
    m_color = color;
}

