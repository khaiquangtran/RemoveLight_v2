#ifndef HARDWARE_H
#define HARDWARE_H
#include <Arduino.h>
#include "./../Utils/SignalType.h"
#include "./../Utils/Package.h"

class Hardware
{
public:
    Hardware();
    virtual ~Hardware() = default;
    virtual void handleSignal(const SignaLType signal, Package *data = nullptr) = 0;
    virtual bool connectWifi() {
        return 1;
    };
    virtual void listen() {
        Serial.println("Listen");
    };
};

#endif // HARDWARE_H