#include "./Timer.h"
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <chrono>
#include <thread> // For sleep_for()
#endif

void Timer::startTimer()
{
    mClear.store(false);

    mStartTime = millis();
    // Schedule task in thread pool after duration
    mPool->enqueue([this]
                   {
        delay(mDuration.load());
        if (!mClear.load()) {
            mFunction();
        } });
}

void Timer::startTimerInterval()
{
    mClear.store(false);
    mPool->enqueue([this]
                   {
        while (!mClear.load()) {

                delay(mDuration.load());
            if (!mClear.load()) {
                mFunction();
            }
        } });
}

void Timer::stopTimer()
{
    mClear.store(true);
    //  Ensure previous task stops
    // #ifdef ARDUINO
    //     delay(mDuration.load() - getElapsedTime() + 10);
    // #else
    //     std::this_thread::sleep_for(std::chrono::milliseconds(mDuration.load() - getElapsedTime() + 10));
    // #endif
}

void Timer::stopTimerInterval() {
    mClear.store(true);
}

void Timer::updateTimer(FuncCallback func, uint16_t duration)
{
    mFunction = std::move(func);
    mDuration.store(duration); // Ensure atomic update
}

uint32_t Timer::getElapsedTime() const
{
    return static_cast<uint32_t>(millis() - mStartTime); // Calculate elapsed time using millis()
}
