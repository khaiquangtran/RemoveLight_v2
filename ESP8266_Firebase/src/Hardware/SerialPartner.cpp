#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(Processor *processor) : mProcessor(processor)
{
    Serial.begin(BAUND_RATE);
    Serial2 = new SoftwareSerial(RXD2, TXD2);
    Serial2->begin(BAUND_RATE);

    mCommandHandle[COMMAND::STATUS_WIFI]            = "0001";
    mCommandHandle[COMMAND::STATUS_FIREBASE]        = "0002";
    mCommandHandle[COMMAND::STATUS_NTP]             = "0003";
    mCommandHandle[COMMAND::WIFI_SUCCESSFULL]       = "1001";
    mCommandHandle[COMMAND::WIFI_FAILED]            = "1002";
    mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL]   = "2002";
    mCommandHandle[COMMAND::FIREBASE_FAILED]        = "2003";
    mCommandHandle[COMMAND::NTP_SUCCESSFULL]        = "3002";
    mCommandHandle[COMMAND::NTP_FAILED]             = "3003";
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

    if (Serial2->available())
    {
        String c = Serial2->readString();
        Serial.println(c.c_str());
        handleMessage(c);
    }
}

void SerialPartner::handleSignal(const SignaLType signal, Package *data)
{
    switch (signal)
    {
    case SignaLType::CONNECT_WIFI_SUCCESSFULL:
    {
        Serial2->write(mCommandHandle[COMMAND::WIFI_SUCCESSFULL].c_str());
        break;
    }
    case SignaLType::CONNECT_FIREBASE_SUCCESSFULL:
    {
        Serial2->write(mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL].c_str());
        break;
    }
    case SignaLType::CONNECT_NTP_SUCCESSFULL:
    {
        Serial2->write(mCommandHandle[COMMAND::NTP_SUCCESSFULL].c_str());
        break;
    }
    case SignaLType::CONNECT_WIFI_FAILED:
    {
        Serial2->write(mCommandHandle[COMMAND::WIFI_FAILED].c_str());
        break;
    }
    case SignaLType::CONNECT_FIREBASE_FAILED:
    {
        Serial2->write(mCommandHandle[COMMAND::FIREBASE_FAILED].c_str());
        break;
    }
    case SignaLType::CONNECT_NTP_FAILED:
    {
        Serial2->write(mCommandHandle[COMMAND::NTP_FAILED].c_str());
        break;
    }
    default:
        break;
    }
}

void SerialPartner::handleMessage(String receiverData)
{
    String command = receiverData.substring(0,4);
    if(command == mCommandHandle[COMMAND::STATUS_WIFI])
    {
        mProcessor->handleSignal(SignaLType::STATUS_WIFI);
    }
    else if(command == mCommandHandle[COMMAND::STATUS_FIREBASE])
    {
        mProcessor->handleSignal(SignaLType::STATUS_FIREBASE);
    }
    else if(command == mCommandHandle[COMMAND::STATUS_NTP])
    {
        mProcessor->handleSignal(SignaLType::STATUS_NTP);
    }
}