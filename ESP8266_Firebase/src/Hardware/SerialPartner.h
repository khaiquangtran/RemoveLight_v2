#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "./Hardware.h"
#include "./../Processor.h"
#include "./../Utils/Package.h"
#include "./../Utils/SignalType.h"
#include <map>

class Processor;

class SerialPartner : public Hardware
{
public:
    SerialPartner(Processor *processor);
	~SerialPartner();
	SerialPartner(const SerialPartner &) = delete;
	SerialPartner &operator=(const SerialPartner &) = delete;

    void handleSignal(const SignaLType signal, Package *data = nullptr);

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
    STATUS_NTP,
    WIFI_SUCCESSFULL,
    WIFI_FAILED,
    FIREBASE_SUCCESSFULL,
    FIREBASE_FAILED,
    NTP_SUCCESSFULL,
    NTP_FAILED,
};

private:
	Processor *mProcessor;
    SoftwareSerial *Serial2;
    const int TXD2 = 5; // D1
    const int RXD2 = 4; // D2
    const int BAUND_RATE = 115200;
    // const int BAUND_RATE_2 = 115200;

    void handleMessage(String receiverData);

    std::map<COMMAND, String> mCommandHandle;
};

#endif // SERIAL_PARTNER_H