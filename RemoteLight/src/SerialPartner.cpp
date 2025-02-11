#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(RemoteLight *rml) : mRML(rml)
{
    Serial.begin(BAUND_RATE);
    Serial2.begin(BAUND_RATE, SERIAL_8N1, RXD2, TXD2);
    mMode = COMMAND::NONE;

    mCommandHandle[COMMAND::STATUS_WIFI]       = "0001";
    mCommandHandle[COMMAND::STATUS_FIREBASE]   = "0002";
    mCommandHandle[COMMAND::STATUS_NTP]        = "0003";

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
        LOGI("%s", receiverData.c_str());
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
            mMode = COMMAND::STATUS_WIFI;
            LOGD("Send STATUS_WIFI");
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
            mMode = COMMAND::STATUS_FIREBASE;
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
        String command = mCommandHandle[COMMAND::STATUS_FIREBASE];
        if(command.length() != 0)
        {
            Serial2.write("STATUS_NTP");
            mMode = COMMAND::STATUS_NTP;
            LOGD("Send STATUS_NTP");
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
    LOGI("Receiver data: %s", receiverData.c_str());
    if(receiverData.substring(0,1) == "1" && mMode == COMMAND::STATUS_WIFI)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS);
        mMode = COMMAND::NONE;
    }
    else if(receiverData.substring(0,1) == "0" && mMode == COMMAND::STATUS_WIFI)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_WIFI_FAILED);
        mMode = COMMAND::NONE;
    }

    if(receiverData.substring(0,1) == "1" && mMode == COMMAND::STATUS_FIREBASE)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS);
        mMode = COMMAND::NONE;
    }
    else if(receiverData.substring(0,1) == "0" && mMode == COMMAND::STATUS_FIREBASE)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_FAILED);
        mMode = COMMAND::NONE;
    }

    if(receiverData.substring(0,1) == "1" && mMode == COMMAND::STATUS_NTP)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS);
        mMode = COMMAND::NONE;
    }
    else if(receiverData.substring(0,1) == "0" && mMode == COMMAND::STATUS_NTP)
    {
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_NTP_FAILED);
        mMode = COMMAND::NONE;
    }
    /*
    else if(mMode == COMMAND::SSID)
    {
        const int length = receiverData.length();
        int data[length];
        for(int i = 0; i < length; i++)
        {
            data[i] = receiverData[i];
        }
        Package package(data, length);
        // mRML->handleSignal(SignaLType::WIFIPARTNER_SEND_SSID, package);
        Serial.println("Enter PASSWORD: ");
        mMode = COMMAND::PASSWORD;

    }
    else if(mMode == COMMAND::PASSWORD)
    {
        Serial.println("Connect wifi ...");
        const int length = receiverData.length();
        int data[length];
        for(int i = 0; i < length; i++)
        {
            data[i] = receiverData[i];
        }
        Package package(data, length);
        // mRML->handleSignal(SignaLType::WIFIPARTNER_SEND_PASSWORD, package);
        mRML->handleSignal(SignaLType::REMOTE_LIGHT_CONNECT_WIFI);
        mMode = COMMAND::NONE;
    }
    else
    {
        Serial.println("Don't support command!!! Please double check.");
    }*/
}