#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include "./Hardware.h"
#include "./RemoteLight.h"

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

enum COMMAND {
    NONE = 0U,
    STATUS_WIFI,
    STATUS_FIREBASE,
    STATUS_NTP,
    SSID,
    PASSWORD,
};

private:
	RemoteLight *mRML;
    COMMAND mMode;
    const int TXD2 = 17;
    const int RXD2 = 16;
    const int BAUND_RATE = 9600;

    void handleMessage(String receiverData);
};

#endif // SERIAL_PARTNER_H