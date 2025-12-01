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
#include <array>
#include <map>
#include "WiFiProv.h"
#include "WiFi.h"


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
    bool mStatusProvision;
    bool mEnableCheckConnectWIFI;

    void signUp();
    void connectWifi();
    void checkConnectNTP();
    void getTimeDataFromNtp();
    void processComboBtnPress();
    static void SysProvEvent(arduino_event_t *sys_event);
    void getSSIDAndPasswordFromEEPROM(const Package *data);
    void uploadDataToFirebase();

    int8_t mCommandAllTimerFlag;

    const std::string FLAG_PATH = "flagCommand/";
    const std::array<String, 4U> HEAD_PATH = {
        "light1", "light2", "light3", "light4",
    };
    const std::array<String, 8U> DATA_PATHS = {
        "/swOn",  "/hourOn",  "/minuteOn",  "/secondOn",
        "/swOff", "/hourOff", "/minuteOff", "/secondOff",
    };

    enum REQUEST_FB : int32_t
    {
        NONE = 0,
        SET_INFORM,
        RECEIVED_INFORM,
        UPLOAD_INFORM,
    };
    std::map<SignalType, int32_t> mSignalLightMap;
    std::map<int32_t, SignalType> mSettingDataLightMap;

    const char * POP = "light1234"; // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
    const char * SERVICE_NAME = "ESP32_LIGHT"; // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
    const char * SERVICE_KEY = NULL; // Password used for SofAP method (NULL = no password needed)
    bool mResetProvisioned;
    uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                      0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };

    static Network* instance;
    const static int8_t MAX_RETRY_PROVISION = 3;
};

#endif // WIFIPARTNER_H