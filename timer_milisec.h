/**
* Created by Piotr Matusz on 22.04.2017.
* This simple timer returns how much time has elapsed since
* creation of timer or usage of reset() method.
* Return type is int, value is calculated in miliseconds.
*
* Sample usage:
*    TimerMSec timer;
*    timer.reset();
*    // operations to be measured
*    int measureTime = timer.elapsed();
**/

#ifndef TIMER_MILISEC_H
#define TIMER_MILISEC_H

#include <chrono>

class TimerMiliSec {

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<int, std::ratio<1, 1000> > milisecond_;
    std::chrono::time_point<clock_> beg_;

public:
    TimerMiliSec() : beg_(clock_::now()) {}
    void reset() {beg_ = clock_::now(); }
    int elapsed() const {
       return std::chrono::duration_cast<milisecond_>(clock_::now() - beg_).count();
    }
};

#endif //TIMER_MILISEC_H
