#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class SerialPartner : public Hardware
{
public:
    SerialPartner(RemoteLight *rml);
    ~SerialPartner();
    SerialPartner(const SerialPartner &) = delete;
    SerialPartner &operator=(const SerialPartner &) = delete;

    void handleSignal(const SignaLType signal, Package *data = nullptr);

    void listenning() override;

    enum COMMAND
    {
        NONE = 0U,
        STATUS_WIFI,
        STATUS_FIREBASE,
        STATUS_NTP,
        WIFI_SUCCESSFULL,
        WIFI_FAILED,
        FIREBASE_SUCCESSFULL,
        FIREBASE_FAILED,
        NTP_SUCCESSFULL,
        NTP_FAILED,
        WEB_GET_ALLTIME_DATA_REQUEST,
        WEB_GET_ALLTIME_DATA_RESPONSE,
        SSID,
        PASSWORD,
    };

private:
    RemoteLight *mRML;
    const int TXD2 = 17;
    const int RXD2 = 16;
    const int BAUD_RATE = 115200;

    void handleMessage(String receiverData);

    std::map<COMMAND, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H