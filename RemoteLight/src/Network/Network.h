#ifndef WIFIPARTNER_H
#define WIFIPARTNER_H

#include <Arduino.h>
#include "./Config.h"
#include "./../RemoteLight.h"
#include "../Utils/SignalType.h"
#include <Firebase_ESP_Client.h>
// #include "addons/TokenHelper.h"
// #include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <array>
#include <map>

class RemoteLight;

class Network
{
public:
    Network(std::shared_ptr<RemoteLight>rml);
    ~Network() {delete mTimeClient;};
    Network(const Network &) = delete;
    Network &operator=(const Network &) = delete;

    void handleSignal(const SignalType signal, Package *data = nullptr);

private:
    std::shared_ptr<RemoteLight>mRML;
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
    void checkCommandFirebase();
    void sendAllTimeDatatoWeb(Package *data);
    void sendLightDataToWeb(Package *data, int lightIndex);
    void sendLightStatusToWeb(Package *data);
    void sendResponseSetLightDatatoWeb();
    void getTimeDataFromNtp();

    int8_t mCommandAllTimerFlag;

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
        IDLE,
        GETTING_ALLTIME_DATA,
        SETTING_ALLTIME_DATA,
        GETTING_LIGHT1_DATA,
        SETTING_LIGHT1_DATA,
        GETTING_LIGHT2_DATA,
        SETTING_LIGHT2_DATA,
        GETTING_LIGHT3_DATA,
        SETTING_LIGHT3_DATA,
        GETTING_LIGHT4_DATA,
        SETTING_LIGHT4_DATA,
        GETTING_ALL_STATUS,
        SETTING_LIGHT1_STATUS,
        SETTING_LIGHT2_STATUS,
        SETTING_LIGHT3_STATUS,
        SETTING_LIGHT4_STATUS,
    };
    std::map<REQUEST_FB, std::pair<SignalType, int>> mRequestSignalMap;
    std::map<SignalType, int> mSignalLightMap;
};

#endif // WIFIPARTNER_H