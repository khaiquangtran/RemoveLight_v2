#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "./Processor.h"
#include "./Package.h"
#include "./Logging.h"
#include "SignalType.h"
#include <map>

class Processor;

class SerialPartner
{
public:
    SerialPartner(Processor *processor);
	~SerialPartner();
	SerialPartner(const SerialPartner &) = delete;
	SerialPartner &operator=(const SerialPartner &) = delete;

    void handleSignal(const uint16_t signal, Package *data = nullptr);

    void listen();

enum CONTROL_MODE {
    NONE,
    GET_COMMAND,
    SSID,
    PASSWORD,
};

enum COMMAND {
    STATUS_WIFI,
    STATUS_FIREBASE,
    STATUS_NTP
};

private:
	Processor *mProcessor;
    SoftwareSerial *Serial2;
    const int TXD2 = 5; // D1
    const int RXD2 = 4; // D2
    const int BAUND_RATE = 9600;
    const int BAUND_RATE_2 = 115200;

    void handleMessage(String receiverData);

    std::map<COMMAND, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H