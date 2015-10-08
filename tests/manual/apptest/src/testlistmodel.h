#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QAbstractListModel>

class TestListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    TestListModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QHash<int,QByteArray> roleNames() const;


signals:
    void countChanged();
    void readyChanged();

private:

};

#endif