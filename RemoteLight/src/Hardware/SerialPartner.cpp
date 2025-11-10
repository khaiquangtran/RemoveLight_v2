#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(std::shared_ptr<RemoteLight> rml) : mRML(rml)
{
    Serial.begin(BAUD_RATE);
    LOGI(" ================== SerialPartner ================== ");
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listenning()
{
    String receiverData;
    while (Serial2.available() > 0) {
        receiverData = Serial2.readString();
        LOGI("%s", receiverData.c_str());
    }
}

void SerialPartner::handleSignal(const SignalType& signal, const Package *data)
{
    switch (signal)
    {
    case (1):{
        break;
    }
    default:
        break;
    }
}
