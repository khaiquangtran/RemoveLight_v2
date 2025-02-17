#ifndef TIMER_MGR_H
#define TIMER_MGR_H

#include "./Timer.h"
#include "./ThreadPool.h"

class TimerManager
{
public:
    TimerManager(size_t numThreads) : pool(std::make_shared<ThreadPool>(numThreads)) {}

    std::shared_ptr<Timer> createTimer(FuncCallback func, uint16_t duration);

private:
    std::shared_ptr<ThreadPool> pool;
};

#endif // TIMER_MGR_H