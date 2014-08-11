#include "qmcloadingmeasurer.h"

#include <QQmlComponent>

#include <iostream>


double CPUTimer::getCPUTime()
{
#if defined(linux)
   struct timespec now;
   if(clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now) == 0) {
       return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
   } else {
       return -1.0;
   }
#else
#waring "Unable to define getCPUTime( ) for this OS."
    return -1.0;
#endif
}

void CPUTimer::start()
{
    m_startTime = getCPUTime();
}

double CPUTimer::elapsed()
{
    return getCPUTime() - m_startTime;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////


void QmcLoadingMeasurer::start()
{
    m_timer.start();
}

void QmcLoadingMeasurer::done()
{
    double elapsed = m_timer.elapsed();
    std::cout << "Loading took " << elapsed << " secs!" << std::endl;
}

void QmcLoadingMeasurer::update()
{
    if(!m_component)
        return;

    if (m_component->progress() == 0.0) {
        m_timer.start();
    } else if (m_component->progress() == 1.0) {
        double elapsed = m_timer.elapsed();
        std::cout << "Loading took " << elapsed << " secs!" << std::endl;
    }
}
