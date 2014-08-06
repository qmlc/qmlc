#ifndef CPUTIMER_H
#define CPUTIMER_H

#ifdef linux

#include <time.h>

class CPUTimer
{
public:
    CPUTimer(clockid_t clk_id) : m_clkId(clk_id)
    {}

    void start()
    {
        m_startTime = get_time();
    }

    double elapsed()
    {
        return get_time() - m_startTime;
    }

private:

    double get_time()
    {
       struct timespec now;
       if(clock_gettime(m_clkId, &now) == 0) {
           return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
       } else {
           return -1.0;
       }
    }

    clockid_t m_clkId;
    double m_startTime;
};

#endif // linux

#endif // CPUTIMER_H
