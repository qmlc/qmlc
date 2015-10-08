#ifndef TESTMODEL_H
#define TESTMODEL_H

#include <QAbstractListModel>
#include <QAudioOutput>

class TestListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    TestListModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QHash<int,QByteArray> roleNames() const;
    Q_INVOKABLE QString getString();
    Q_INVOKABLE int getIntValue();

signals:
    void countChanged();
    void readyChanged();

private:

};

class TestListModel2 : public TestListModel
{
    Q_OBJECT

public:
    TestListModel2(QObject *parent = 0);
    Q_INVOKABLE QString getString2();
    Q_INVOKABLE int getIntValue2();
    
}; 

class TestAudioOutput : public QAudioOutput {
    Q_OBJECT
public:
    TestAudioOutput();
};

#endif
