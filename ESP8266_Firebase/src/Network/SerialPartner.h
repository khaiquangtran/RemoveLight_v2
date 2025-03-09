#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "./Network.h"
#include "./../Processor.h"
#include <map>

class Processor;

class SerialPartner : public Network
{
public:
    SerialPartner(Processor *processor);
    ~SerialPartner();
    SerialPartner(const SerialPartner &) = delete;
    SerialPartner &operator=(const SerialPartner &) = delete;

    void handleSignal(const SignalType signal, Package *data = nullptr) override;

    void listen();

private:
    Processor *mProcessor;
    const int TXD2 = 5; // D1
    const int RXD2 = 4; // D2
    const int BAUD_RATE = 115200;
    // const int BAUD_RATE = 115200;
    SoftwareSerial *mSerial2;
    void handleMessage(String receiverData);

    std::map<SignalType, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H