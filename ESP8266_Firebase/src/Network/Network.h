#ifndef NETWORK_H
#define NETWORK_H
#include <Arduino.h>
#include "./../Utils/SignalType.h"
#include "./../Utils/Package.h"

class Network
{
public:
    Network();
    virtual ~Network() = default;
    virtual void handleSignal(const SignalType signal, Package *data = nullptr) = 0;
    virtual void listen() {
        Serial.println("Listen");
    };

protected:
    int *parseCommandStringToArray(String str, int &size);
};

#endif // NETWORK_H