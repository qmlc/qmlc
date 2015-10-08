#include "testlistmodel.h"



TestListModel::TestListModel(QObject *parent): QAbstractListModel(parent) {

}

int TestListModel::rowCount(const QModelIndex &parent) const {

    return 3;
}

QVariant TestListModel::data(const QModelIndex &index, int role) const {

    QString str = "pd2osj";
    QVariant var(str);
    return var;
}
QHash<int,QByteArray> TestListModel::roleNames() const {

    QHash<int,QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    return roles;
}

QString TestListModel::getString() {

    return "from TestListModel";
}
int TestListModel::getIntValue() {
    return 11;
}


TestListModel2::TestListModel2(QObject *parent) : TestListModel(parent) {

} 

QString TestListModel2::getString2() {

    return "from TestListModel2";
}
int TestListModel2::getIntValue2() {
    return 12;
}

TestAudioOutput::TestAudioOutput() : QAudioOutput() {
}
