#include <Arduino.h>
#include "./SerialPartner.h"

SerialPartner::SerialPartner(RemoteLight *rml) : mRML(rml)
{
    Serial.begin(BAUD_RATE);
    Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2);

    mCommandHandle[SignaLType::SERIAL_CHECK_STATUS_WIFI]                = "0001";
    mCommandHandle[SignaLType::SERIAL_CHECK_STATUS_FIREBASE]            = "0002";
    mCommandHandle[SignaLType::SERIAL_CHECK_STATUS_NTP]                 = "0003";

    mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS]       = "1001";
    mCommandHandle[SignaLType::WIFI_FAILED]                             = "1002";
    mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS]   = "2002";
    mCommandHandle[SignaLType::FIREBASE_FAILED]                         = "2003";
    mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS]        = "3002";
    mCommandHandle[SignaLType::NTP_FAILED]                              = "3003";
    mCommandHandle[SignaLType::WEB_GET_ALLTIME_DATA_REQUEST]            = "4004";
    mCommandHandle[SignaLType::WEB_GET_ALLTIME_DATA_RESPONSE]           = "4005";
    mCommandHandle[SignaLType::WEB_SET_ALLTIME_DATA_REQUEST]            = "4006";
    mCommandHandle[SignaLType::WEB_SET_ALLTIME_DATA_RESPONSE]           = "4007";

    mCommandHandle[SignaLType::WEB_GET_LIGHT1_DATA_REQUEST]             = "4104";
    mCommandHandle[SignaLType::WEB_GET_LIGHT1_DATA_RESPONSE]            = "4105";
    mCommandHandle[SignaLType::WEB_SET_LIGHT1_DATA_REQUEST]             = "4106";
    mCommandHandle[SignaLType::WEB_SET_LIGHT1_DATA_RESPONSE]            = "4107";

    mCommandHandle[SignaLType::WEB_GET_LIGHT2_DATA_REQUEST]             = "4204";
    mCommandHandle[SignaLType::WEB_GET_LIGHT2_DATA_RESPONSE]            = "4205";
    mCommandHandle[SignaLType::WEB_SET_LIGHT2_DATA_REQUEST]             = "4206";
    mCommandHandle[SignaLType::WEB_SET_LIGHT2_DATA_RESPONSE]            = "4207";

    mCommandHandle[SignaLType::WEB_GET_LIGHT3_DATA_REQUEST]             = "4304";
    mCommandHandle[SignaLType::WEB_GET_LIGHT3_DATA_RESPONSE]            = "4305";
    mCommandHandle[SignaLType::WEB_SET_LIGHT3_DATA_REQUEST]             = "4306";
    mCommandHandle[SignaLType::WEB_SET_LIGHT3_DATA_RESPONSE]            = "4307";

    mCommandHandle[SignaLType::WEB_GET_LIGHT4_DATA_REQUEST]             = "4404";
    mCommandHandle[SignaLType::WEB_GET_LIGHT4_DATA_RESPONSE]            = "4405";
    mCommandHandle[SignaLType::WEB_SET_LIGHT4_DATA_REQUEST]             = "4406";
    mCommandHandle[SignaLType::WEB_SET_LIGHT4_DATA_RESPONSE]            = "4407";

    mCommandHandle[SignaLType::WEB_GET_STATUS_DATA_REQUEST]             = "4504";
    mCommandHandle[SignaLType::WEB_GET_STATUS_DATA_RESPONSE]            = "4505";
    mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST]      = "4516";
    mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST]      = "4526";
    mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST]      = "4536";
    mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST]      = "4546";
    mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE]      = "4507";

    mCommandHandle[SignaLType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP]     = "5000";
    mCommandHandle[SignaLType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP]    = "5001";

    LOGI("Initialization SerialPartner!");
}

SerialPartner::~SerialPartner()
{
}

void SerialPartner::listenning()
{
    String receiverData;
    while (Serial2.available() > 0) {
        receiverData = Serial2.readString();
        // LOGI("%s", receiverData.c_str());
        handleMessage(receiverData);
    }
}

void SerialPartner::handleSignal(const SignaLType signal, Package *data)
{
    switch (signal)
    {
    case (SignaLType::SERIAL_CHECK_STATUS_FIREBASE):
    case (SignaLType::SERIAL_CHECK_STATUS_NTP):
    case (SignaLType::SERIAL_CHECK_STATUS_WIFI):
    case (SignaLType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP):
    {
        if (mCommandHandle.find(signal) != mCommandHandle.end()) {
            Serial2.write(mCommandHandle[signal].c_str());
        }
        else {
            LOGE("Command does not exist!)");
        }
        break;
    }
    case (SignaLType::WEB_GET_ALLTIME_DATA_RESPONSE):
    case (SignaLType::WEB_GET_LIGHT1_DATA_RESPONSE):
    case (SignaLType::WEB_GET_LIGHT2_DATA_RESPONSE):
    case (SignaLType::WEB_GET_LIGHT3_DATA_RESPONSE):
    case (SignaLType::WEB_GET_LIGHT4_DATA_RESPONSE):
    case (SignaLType::WEB_GET_STATUS_DATA_RESPONSE):
    {
        if (mCommandHandle.find(signal) != mCommandHandle.end()) {
            int *pack = data->getPackage();
            String command = mCommandHandle[signal];
            for(int i = 0; i < data->getSize(); i++)
            {
                command += String(" ") + String(pack[i]);
            }
            Serial2.write(command.c_str());
        }
        else {
            LOGE("Command does not exist!)");
        }
        break;
    }
    case (SignaLType::WEB_SET_ALLTIME_DATA_RESPONSE):
    case (SignaLType::WEB_SET_LIGHT1_DATA_RESPONSE):
    case (SignaLType::WEB_SET_LIGHT2_DATA_RESPONSE):
    case (SignaLType::WEB_SET_LIGHT3_DATA_RESPONSE):
    case (SignaLType::WEB_SET_LIGHT4_DATA_RESPONSE):
    case (SignaLType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE):
    {
        if (mCommandHandle.find(signal) != mCommandHandle.end()) {
            Serial2.write(mCommandHandle[signal].c_str());
        }
        else {
            LOGE("Command does not exist!)");
        }
        break;
    }
    default:
        break;
    }
}

void SerialPartner::handleMessage(String receiverData)
{
    String command = receiverData.substring(0, 4);
    LOGI("Receiver data: %s", command);

    if (command == mCommandHandle[SignaLType::WEB_GET_ALLTIME_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_GET_LIGHT1_DATA_REQUEST] ||
        command == mCommandHandle[SignaLType::WEB_GET_LIGHT2_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_GET_LIGHT3_DATA_REQUEST] ||
        command == mCommandHandle[SignaLType::WEB_GET_LIGHT4_DATA_REQUEST] || command == mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS] ||
        command == mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS]  || command == mCommandHandle[SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS] ||
        command == mCommandHandle[SignaLType::WEB_GET_STATUS_DATA_REQUEST])
    {
        std::map<SignaLType, String>::iterator it;
        for( it = mCommandHandle.begin(); it != mCommandHandle.end(); it++)
        {
            if (it->second == command)
            {
                mRML->handleSignal(it->first);
                break;
            }
        }
    }
    else if (command == mCommandHandle[SignaLType::WEB_SET_LIGHT4_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_SET_LIGHT3_DATA_REQUEST] ||
             command == mCommandHandle[SignaLType::WEB_SET_LIGHT2_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_SET_LIGHT1_DATA_REQUEST] ||
             command == mCommandHandle[SignaLType::WEB_SET_ALLTIME_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST] ||
             command == mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST] || command == mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST] ||
             command == mCommandHandle[SignaLType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST] || command == mCommandHandle[SignaLType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP])
    {
        std::map<SignaLType, String>::iterator it;
        for( it = mCommandHandle.begin(); it != mCommandHandle.end(); it++)
        {
            if (it->second == command)
            {
                int size = 0;
                LOGD(receiverData.c_str());
                int* data = parseCommandStringToArray(receiverData, size);
                Package* package = new Package(data, size);
                mRML->handleSignal(it->first, package);
                delete[] data;
                delete package;
                break;
            }
        }
    }
    else if (command == mCommandHandle[SignaLType::WIFI_FAILED]) {
        LOGW("Received wifi connection signal failed. Try again.");
    }
    else if (command == mCommandHandle[SignaLType::FIREBASE_FAILED]) {
        LOGW("Received Firebase connection signal failed. Try again.");
    }
    else if (command == mCommandHandle[SignaLType::NTP_FAILED]) {
        LOGW("Received NTP connection signal failed. Try again.");
    }
    else {
        // Do nothing
    }
}