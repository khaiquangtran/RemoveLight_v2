#ifndef WIFIPARTNER_H
#define WIFIPARTNER_H

#include <Arduino.h>
#include "./Config.h"
#include "./Network.h"
#include "./../Processor.h"
#include <Firebase_ESP_Client.h>
// #include "addons/TokenHelper.h"
// #include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <array>
#include <map>

class Processor;

class WifiPartner : public Network
{
public:
    WifiPartner(Processor *processor);
    ~WifiPartner() {};
    WifiPartner(const WifiPartner &) = delete;
    WifiPartner &operator=(const WifiPartner &) = delete;

    void handleSignal(const SignalType signal, Package *data = nullptr) override;

private:
    Processor *mProcessor;
    String mSSID;
    String mPassword;
    FirebaseData mFbdo;
    FirebaseAuth mAuth;
    FirebaseConfig mConfig;
    const uint32_t GMT = 25200; // GMT +7
    WiFiUDP ntpUDP;
    NTPClient *mTimeClient;

    void signUp();
    void connectWifi();
    void checkConnectNTP();
    void checkAlltimeCommandFirebase();
    void sendAllTimeDatatoWeb(Package *data);
    void sendLightDataToWeb(Package *data, int lightIndex);
    void sendLightStatusToWeb(Package *data);
    void sendResponseSetLightDatatoWeb(int lightIndex);
    void checkLightCommandFromFirebase(SignalType signalGetRequest, SignalType signalSetRequest, int lightIndex);
    void checkStatusCommandFromFirebase();
    void getTimeDataFromNtp();

    int8_t mCommandAllTimerFlag;
    std::map<SignalType, int> mSignalLightMap;

    const std::array<String, 8U> ALLTIME_PATH = {
        "allTime/command",  "allTime/data/hour", "allTime/data/minute", "allTime/data/second",
        "allTime/data/day", "allTime/data/date", "allTime/data/month",  "allTime/data/year"
    };
    const std::array<String, 6U> LIGHT_PATHS = {
        "allTime", "light1", "light2", "light3", "light4", "status"
    };
    const std::array<String, 13U> DATA_PATHS = { "/command",
        "/data/swOn",  "/data/hourOn",  "/data/minuteOn",  "/data/secondOn",
        "/data/swOff", "/data/hourOff", "/data/minuteOff", "/data/secondOff",
        "/data/light1", "/data/light2", "/data/light3", "/data/light4",
    };

    enum REQUEST_FB : int
    {
        SENT_INFORM = 0,
        GETTING,
        SETTING,
        IDLE,
        SETTING_LIGHT1,
        SETTING_LIGHT2,
        SETTING_LIGHT3,
        SETTING_LIGHT4,
    };

    std::map<REQUEST_FB, std::pair<SignalType, int>> mRequestSignalMap;
};

#endif // WIFIPARTNER_H