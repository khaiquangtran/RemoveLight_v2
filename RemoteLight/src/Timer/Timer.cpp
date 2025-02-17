#include "./Timer.h"
#include <Arduino.h>

void Timer::startTimer()
{
    mClear.store(false);

    // Schedule task in thread pool after duration
    mPool->enqueue([this]
                   {
        delay(mDuration);

        if (!mClear.load()) {
            mFunction();
        } });
}

void Timer::stopTimer()
{
    mClear.store(true);
}