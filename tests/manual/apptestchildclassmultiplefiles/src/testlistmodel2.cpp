#include "testlistmodel2.h"

TestListModel2::TestListModel2(QObject *parent) : TestListModel(parent) {

}

QString TestListModel2::getString2() {

    return "really from TestListModel2";
}
int TestListModel2::getIntValue2() {
    return 102;
}
