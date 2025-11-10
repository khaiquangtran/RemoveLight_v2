#ifndef SIGNAL_TO_STRING_H
#define SIGNAL_TO_STRING_H

#include "SignalType.h"

inline String SIGNALTOSTRING(const SignalType& signal)
{
    switch (signal) {
        case SignalType::TASKS_CONNECT_WIFI:
            return "TASKS_CONNECT_WIFI";
        case SignalType::TASKS_CONNECT_FIREBASE:
            return "TASKS_CONNECT_FIREBASE";
        case SignalType::TASKS_CONNECT_NTP:
            return "TASKS_CONNECT_NTP";
        case SignalType::TASKS_DISPLAY_ALL_TIME:
            return "TASKS_DISPLAY_ALL_TIME";
        case SignalType::TASKS_START_SETUP_MODE:
            return "TASKS_START_SETUP_MODE";
        case SignalType::TASKS_END_SETUP_MODE:
            return "TASKS_END_SETUP_MODE";
        default:
            return "UNKNOWN_SIGNAL";
    }
}

#endif // SIGNAL_TO_STRING_H