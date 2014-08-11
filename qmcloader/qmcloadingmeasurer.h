#ifndef QMCLOADINGMEASURER_H
#define QMCLOADINGMEASURER_H


#include <QObject>

#if defined(linux)
#include <time.h>
#endif


class QQmlComponent;


class CPUTimer
{
public:
    CPUTimer()
    {}

    void start();
    double elapsed();

private:

    double getCPUTime();

    double m_startTime;
};


class QmcLoadingMeasurer : public QObject
{
    Q_OBJECT

public:
    QmcLoadingMeasurer(QQmlComponent* component = 0) :
        m_component(component)
    {}

    void start();
    void done();

public slots:

    void update();

private:
    QQmlComponent* m_component;
    CPUTimer m_timer;
};


#endif // QMCLOADINGMEASURER_H
