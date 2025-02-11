#include "./SerialPartner.h"

SerialPartner::SerialPartner(Processor *processor) : mProcessor(processor)
{
    Serial.begin(BAUND_RATE_2);
    Serial2 = new SoftwareSerial(RXD2, TXD2);
    Serial2->begin(BAUND_RATE);

    mCommandHandle[COMMAND::STATUS_WIFI]       = "0001";
    mCommandHandle[COMMAND::STATUS_FIREBASE]   = "0002";
    mCommandHandle[COMMAND::STATUS_NTP]        = "0003";

    LOGI("Initialization Serial Partner");
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listen()
{
    String receiverData;
    if (Serial.available())
    {
        String c = Serial.readString();
        Serial2->print(c);
    }

    // Nếu có dữ liệu từ cổng nối tiếp mềm, gửi nó sang cổng nối tiếp cứng
    if (Serial2->available())
    {
        String c = Serial2->readString();
        Serial.print(c);
        handleMessage(c);
    }
}

void SerialPartner::handleSignal(const uint16_t signal, Package *data)
{
    switch (signal)
    {
    case 1:
    {
        break;
    }
    default:
        break;
    }
}

void SerialPartner::handleMessage(String receiverData)
{
    LOGI("Receiver data: %s", receiverData.c_str());
    if(receiverData == mCommandHandle[COMMAND::STATUS_WIFI])
    {
        mProcessor->handleSignal(SignaLType::STATUS_WIFI);
    }
}