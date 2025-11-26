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
    #ifdef ARDUINO
        mStartTime =  millis();
    #else
        mStartTime = std::chrono::steady_clock::now();
    #endif
    // Schedule task in thread pool after duration
    mPool->enqueue([this]
                   {
        #ifdef ARDUINO
            delay(mDuration.load());
        #else
            std::this_thread::sleep_for(std::chrono::milliseconds(mDuration.load()));
        #endif
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
            #ifdef ARDUINO
                delay(mDuration.load());
            #else
                std::this_thread::sleep_for(std::chrono::milliseconds(mDuration.load()));
            #endif
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
    #ifdef ARDUINO
        return static_cast<uint32_t>(millis() - mStartTime); // Calculate elapsed time using millis()
    #else
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - mStartTime);
        return static_cast<uint32_t>(elapsed.count());
    #endif
}
