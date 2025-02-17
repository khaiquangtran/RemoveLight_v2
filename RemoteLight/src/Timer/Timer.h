#ifndef TIMER_H
#define TIMER_H
#include "./ThreadPool.h"

class Timer
{
public:
    Timer(std::shared_ptr<ThreadPool> pool, FuncCallback func, uint16_t duration) :
        mPool(pool),
        mClear(false),
        mFunction(std::move(func)),
        mDuration(duration) {}

    void startTimer();
    void stopTimer();

private:
    std::shared_ptr<ThreadPool> mPool;
    std::atomic<bool> mClear;
    std::function<void()> mFunction;
    uint16_t mDuration;
};

#endif // TIMER_H