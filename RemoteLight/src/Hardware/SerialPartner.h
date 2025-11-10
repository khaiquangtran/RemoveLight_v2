#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include "./Hardware.h"
#include "./../RemoteLight.h"

class RemoteLight;

class SerialPartner : public Hardware
{
public:
    SerialPartner(std::shared_ptr<RemoteLight> rml);
    ~SerialPartner();
    SerialPartner(const SerialPartner &) = delete;
    SerialPartner &operator=(const SerialPartner &) = delete;

    void handleSignal(const SignalType& signal, const Package* data = nullptr);

    void listenning() override;

private:
    std::shared_ptr<RemoteLight> mRML;
    const int32_t TXD2 = 17;
    const int32_t RXD2 = 16;
    const int32_t BAUD_RATE = 115200;

    void handleMessage(String receiverData);

    std::map<SignalType, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H