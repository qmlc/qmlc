/*!
 * Copyright (C) 2014 Nomovok Ltd. All rights reserved.
 * Contact: info@nomovok.com
 *
 * This file may be used under the terms of the GNU Lesser
 * General Public License version 2.1 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU Lesser General Public License version 2.1 requirements
 * will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 *
 * In addition, as a special exception, copyright holders
 * give you certain additional rights.  These rights are described in
 * the Digia Qt LGPL Exception version 1.1, included in the file
 * LGPL_EXCEPTION.txt in this package.
 */

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
#warning "Unable to define getCPUTime( ) for this OS."
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
