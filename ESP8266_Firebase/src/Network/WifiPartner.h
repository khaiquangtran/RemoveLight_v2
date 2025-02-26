#ifndef WIFIPARTNER_H
#define WIFIPARTNER_H

#include <Arduino.h>
#include "./Config.h"
#include "./Network.h"
#include "./../Processor.h"
#include <Firebase_ESP_Client.h>
// #include <addons/TokenHelper.h>
// #include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

class Processor;

class WifiPartner : public Network
{
public:
    WifiPartner(Processor *processor);
    ~WifiPartner() {};
    WifiPartner(const WifiPartner &) = delete;
    WifiPartner &operator=(const WifiPartner &) = delete;

    void handleSignal(const SignalType signal, Package *data = nullptr) override;

    // bool getStatusWifi() { return mStatusWifi; }

private:
    Processor *mProcessor;
    String mSSID;
    String mPassword;
    FirebaseData mFbdo;
    FirebaseAuth mAuth;
    FirebaseConfig mConfig;
    const uint32_t GMT = 28800;
    WiFiUDP ntpUDP;
    NTPClient *mTimeClient;

    void signUp();
    void connectWifi();
    void checkConnectNTP();
    void checkCommandFirebase();
    void sendAllTimeDatatoWeb(Package *data);

    int8_t mCommandAllTimerFlag;
};

#endif // WIFIPARTNER_H