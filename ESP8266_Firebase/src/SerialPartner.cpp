#include "./SerialPartner.h"

SerialPartner::SerialPartner(Processor *processor) : mProcessor(processor)
{
    Serial.begin(BAUND_RATE);
    Serial.println("Initialization Serial Partner");
    Serial2 = new SoftwareSerial(RXD2, TXD2);
    Serial2->begin(BAUND_RATE);
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
        Serial2->println(c);
    }

    // Nếu có dữ liệu từ cổng nối tiếp mềm, gửi nó sang cổng nối tiếp cứng
    if (Serial2->available())
    {
        String c = Serial2->readString();
        Serial.println(c);
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
}