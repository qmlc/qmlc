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

    return "really from TestListModel";
}
int TestListModel::getIntValue() {
    return 101;
}


