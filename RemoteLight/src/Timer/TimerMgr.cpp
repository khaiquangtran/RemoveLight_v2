#include "./Timer/TimerMgr.h"

std::shared_ptr<Timer> TimerManager::createTimer(FuncCallback func, uint16_t duration) {
    return std::make_shared<Timer>(pool, func, duration);
}