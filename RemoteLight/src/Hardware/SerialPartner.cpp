#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(RemoteLight *rml) : mRML(rml)
{
    Serial.begin(BAUD_RATE);
    Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2);

    mCommandHandle[COMMAND::STATUS_WIFI]            = "0001";
    mCommandHandle[COMMAND::STATUS_FIREBASE]        = "0002";
    mCommandHandle[COMMAND::STATUS_NTP]             = "0003";
    mCommandHandle[COMMAND::WIFI_SUCCESSFULL]       = "1001";
    mCommandHandle[COMMAND::WIFI_FAILED]            = "1002";
    mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL]   = "2002";
    mCommandHandle[COMMAND::FIREBASE_FAILED]        = "2003";
    mCommandHandle[COMMAND::NTP_SUCCESSFULL]        = "3002";
    mCommandHandle[COMMAND::NTP_FAILED]             = "3003";
    mCommandHandle[COMMAND::REQUEST_ALLTIME_DATA]   = "4004";

    LOGI("Initialization SerialPartner!");
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listenning()
{
    String receiverData;
    while (Serial2.available() > 0)
    {
        receiverData = Serial2.readString();
        // LOGI("%s", receiverData.c_str());
        handleMessage(receiverData);
    }
}

void SerialPartner::handleSignal(const SignaLType signal, Package *data)
{
    switch (signal)
    {
    case (SignaLType::SERIAL_CHECK_STATUS_WIFI):
    {
        String command = mCommandHandle[COMMAND::STATUS_WIFI];
        if(command.length() != 0)
        {
            Serial2.write(command.c_str());
            // LOGD("Send the command to connect to WiFi");
        }
        else
        {
            LOGD("Command STATUS_WIFI is incomplete!)");
        }
        break;
    }
    case (SignaLType::SERIAL_CHECK_STATUS_FIREBASE):
    {
        String command = mCommandHandle[COMMAND::STATUS_FIREBASE];
        if(command.length() != 0)
        {
            Serial2.write(command.c_str());
            LOGD("Send STATUS_FIREBASE");
        }
        else
        {
            LOGD("Command STATUS_FIREBASE is incomplete!)");
        }
        break;
    }
    case (SignaLType::SERIAL_CHECK_STATUS_NTP):
    {
        String command = mCommandHandle[COMMAND::STATUS_NTP];
        if(command.length() != 0)
        {
            Serial2.write(command.c_str());
            // LOGD("Send STATUS_NTP");
        }
        else
        {
            LOGD("Command STATUS_NTP is incomplete!)");
        }
        break;
    }
    default: break;
    }
}

void SerialPartner::handleMessage(String receiverData)
{
    String command = receiverData.substring(0,4);
    LOGI("Receiver data: %s", command);
    if(command == mCommandHandle[COMMAND::WIFI_SUCCESSFULL])
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS);
    }
    else if(command == mCommandHandle[COMMAND::WIFI_FAILED])
    {
        LOGW("Received wifi connection signal failed. Try again.");
    }
    else if(command == mCommandHandle[COMMAND::FIREBASE_SUCCESSFULL])
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS);
    }
    else if(command == mCommandHandle[COMMAND::FIREBASE_FAILED])
    {
        LOGW("Received Firebase connection signal failed. Try again.");
    }
    else if(command == mCommandHandle[COMMAND::NTP_SUCCESSFULL])
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS);
    }
    else if(command == mCommandHandle[COMMAND::NTP_FAILED])
    {
        LOGW("Received NTP connection signal failed. Try again.");
    }
    else if(command == mCommandHandle[COMMAND::REQUEST_ALLTIME_DATA])
    {
        mRML->handleSignal(SignaLType::REQUEST_ALLTIME_DATA);
    }
}