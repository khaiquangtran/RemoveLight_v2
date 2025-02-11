#ifndef WIFIPARTNER_H
#define WIFIPARTNER_H

#include <Arduino.h>
#include "./WifiConfig.h"
#include "./Package.h"

class Processor;

class WifiPartner
{
public:
    WifiPartner(Processor *processor);
    ~WifiPartner() {};
    WifiPartner(const WifiPartner &) = delete;
    WifiPartner &operator=(const WifiPartner &) = delete;

    void handleSignal(const uint16_t signal, Package *data = nullptr);

    bool connectWifi();
    bool getStatusWifi() { return mStatusWifi; }

private:
    Processor *mProcessor;
    String mSSID;
    String mPassword;

    bool mStatusWifi;
};

#endif // WIFIPARTNER_H