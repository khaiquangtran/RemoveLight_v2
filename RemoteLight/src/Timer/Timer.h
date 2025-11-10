#ifndef TIMER_H
#define TIMER_H
#include "./ThreadPool.h"
#ifdef ARDUINO
#include <Arduino.h>
typedef uint32_t TimePoint; // Use millis() on ESP32
#else
#include <chrono>
typedef std::chrono::steady_clock::time_point TimePoint; // Use steady_clock on Win32
#endif

class Timer
{
public:
    Timer(std::shared_ptr<ThreadPool> pool, FuncCallback func, uint16_t duration) :
        mPool(pool),
        mClear(false),
        mFunction(std::move(func)),
        mDuration(duration) {}

    void startTimer();
    void startTimerInterval();
    void stopTimer();
    void stopTimerInterval();
    void updateTimer(FuncCallback func, uint16_t duration);
    uint32_t getElapsedTime() const;
private:
    std::shared_ptr<ThreadPool> mPool;
    std::atomic<bool> mClear;
    std::function<void()> mFunction;
    std::atomic<uint16_t> mDuration;
    TimePoint mStartTime;
};

#endif // TIMER_H