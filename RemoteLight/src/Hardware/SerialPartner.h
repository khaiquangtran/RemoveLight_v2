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

    void handleSignal(const SignalType signal, Package *data = nullptr);

    void listenning() override;

private:
    RemoteLight *mRML;
    const int TXD2 = 17;
    const int RXD2 = 16;
    const int BAUD_RATE = 115200;

    void handleMessage(String receiverData);

    std::map<SignalType, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H