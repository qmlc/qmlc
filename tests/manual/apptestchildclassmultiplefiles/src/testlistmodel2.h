#ifndef TESTMODEL2_H
#define TESTMODEL2_H

#include "testlistmodel.h"

class TestListModel2 : public TestListModel
{
    Q_OBJECT

public:
    TestListModel2(QObject *parent = 0);
    Q_INVOKABLE QString getString2();
    Q_INVOKABLE int getIntValue2();
};

#endif