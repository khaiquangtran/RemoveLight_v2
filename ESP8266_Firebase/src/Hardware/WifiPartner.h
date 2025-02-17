#ifndef WIFIPARTNER_H
#define WIFIPARTNER_H

#include <Arduino.h>
#include "./WifiConfig.h"
#include "./Hardware.h"
#include "./../Utils/Package.h"
#include "./../Utils/SignalType.h"
#include "./../Processor.h"

class Processor;

class WifiPartner : public Hardware
{
public:
    WifiPartner(Processor *processor);
    ~WifiPartner() {};
    WifiPartner(const WifiPartner &) = delete;
    WifiPartner &operator=(const WifiPartner &) = delete;

    void handleSignal(const SignaLType signal, Package *data = nullptr) {};

    bool connectWifi();
    bool getStatusWifi() { return mStatusWifi; }

private:
    Processor *mProcessor;
    String mSSID;
    String mPassword;

    bool mStatusWifi;
};

#endif // WIFIPARTNER_H