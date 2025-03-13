#include "./SerialPartner.h"

SerialPartner::SerialPartner(Processor *processor) : mProcessor(processor)
{
    Serial.begin(BAUD_RATE);
    mSerial2 = new SoftwareSerial(RXD2, TXD2);
    mSerial2->begin(BAUD_RATE);

    mCommandHandle[SignalType::STATUS_WIFI]                             = "0001";
    mCommandHandle[SignalType::STATUS_FIREBASE]                         = "0002";
    mCommandHandle[SignalType::STATUS_NTP]                              = "0003";
    mCommandHandle[SignalType::CONNECT_WIFI_SUCCESSFULL]                = "1001";
    mCommandHandle[SignalType::CONNECT_WIFI_FAILED]                     = "1002";
    mCommandHandle[SignalType::CONNECT_RETRY]                           = "1003";
    mCommandHandle[SignalType::CONNECT_FIREBASE_SUCCESSFULL]            = "2002";
    mCommandHandle[SignalType::CONNECT_FIREBASE_FAILED]                 = "2003";
    mCommandHandle[SignalType::CONNECT_NTP_SUCCESSFULL]                 = "3002";
    mCommandHandle[SignalType::CONNECT_NTP_FAILED]                      = "3003";

    mCommandHandle[SignalType::WEB_GET_ALLTIME_DATA_REQUEST]            = "4004";
    mCommandHandle[SignalType::WEB_GET_ALLTIME_DATA_RESPONSE]           = "4005";
    mCommandHandle[SignalType::WEB_SET_ALLTIME_DATA_REQUEST]            = "4006";
    mCommandHandle[SignalType::WEB_SET_ALLTIME_DATA_RESPONSE]           = "4007";

    mCommandHandle[SignalType::WEB_GET_LIGHT1_DATA_REQUEST]             = "4104";
    mCommandHandle[SignalType::WEB_GET_LIGHT1_DATA_RESPONSE]            = "4105";
    mCommandHandle[SignalType::WEB_SET_LIGHT1_DATA_REQUEST]             = "4106";
    mCommandHandle[SignalType::WEB_SET_LIGHT1_DATA_RESPONSE]            = "4107";

    mCommandHandle[SignalType::WEB_GET_LIGHT2_DATA_REQUEST]             = "4204";
    mCommandHandle[SignalType::WEB_GET_LIGHT2_DATA_RESPONSE]            = "4205";
    mCommandHandle[SignalType::WEB_SET_LIGHT2_DATA_REQUEST]             = "4206";
    mCommandHandle[SignalType::WEB_SET_LIGHT2_DATA_RESPONSE]            = "4207";

    mCommandHandle[SignalType::WEB_GET_LIGHT3_DATA_REQUEST]             = "4304";
    mCommandHandle[SignalType::WEB_GET_LIGHT3_DATA_RESPONSE]            = "4305";
    mCommandHandle[SignalType::WEB_SET_LIGHT3_DATA_REQUEST]             = "4306";
    mCommandHandle[SignalType::WEB_SET_LIGHT3_DATA_RESPONSE]            = "4307";

    mCommandHandle[SignalType::WEB_GET_LIGHT4_DATA_REQUEST]             = "4404";
    mCommandHandle[SignalType::WEB_GET_LIGHT4_DATA_RESPONSE]            = "4405";
    mCommandHandle[SignalType::WEB_SET_LIGHT4_DATA_REQUEST]             = "4406";
    mCommandHandle[SignalType::WEB_SET_LIGHT4_DATA_RESPONSE]            = "4407";

    mCommandHandle[SignalType::WEB_GET_STATUS_DATA_REQUEST]             = "4504";
    mCommandHandle[SignalType::WEB_GET_STATUS_DATA_RESPONSE]            = "4505";
    mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST]      = "4516";
    mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST]      = "4526";
    mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST]      = "4536";
    mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST]      = "4546";
    mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE]      = "4507";

    mCommandHandle[SignalType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP]     = "5000";
    mCommandHandle[SignalType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP]    = "5001";
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
        mSerial2->print(c);
    }

    if (mSerial2->available())
    {
        String c = mSerial2->readString();
        Serial.println(c.c_str());
        handleMessage(c);
    }
}

void SerialPartner::handleSignal(const SignalType signal, Package *data)
{
    switch (signal)
    {
    case SignalType::CONNECT_WIFI_SUCCESSFULL:
    case SignalType::CONNECT_FIREBASE_SUCCESSFULL:
    case SignalType::CONNECT_NTP_SUCCESSFULL:
    case SignalType::CONNECT_WIFI_FAILED:
    case SignalType::CONNECT_FIREBASE_FAILED:
    case SignalType::CONNECT_NTP_FAILED:
    case SignalType::WEB_GET_ALLTIME_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT4_DATA_REQUEST:
    case SignalType::WEB_GET_STATUS_DATA_REQUEST:
    case SignalType::CONNECT_RETRY:
    {
        mSerial2->write(mCommandHandle[signal].c_str());
        break;
    }
    case SignalType::WEB_SET_ALLTIME_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT4_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST:
    case SignalType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP:
    {
        int *pack = data->getPackage();
        if(pack != nullptr)
        {
            String command = mCommandHandle[signal];
            for(int i = 0; i < data->getSize(); i++)
            {
                command += String(" ") + String(pack[i]);
            }
            mSerial2->write(command.c_str());
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

    if (command == mCommandHandle[SignalType::STATUS_NTP] || command == mCommandHandle[SignalType::STATUS_FIREBASE] ||
        command == mCommandHandle[SignalType::STATUS_WIFI] || command == mCommandHandle[SignalType::WEB_SET_ALLTIME_DATA_RESPONSE] ||
        command == mCommandHandle[SignalType::WEB_SET_LIGHT1_DATA_RESPONSE] || command == mCommandHandle[SignalType::WEB_SET_LIGHT2_DATA_RESPONSE] ||
        command == mCommandHandle[SignalType::WEB_SET_LIGHT3_DATA_RESPONSE] || command == mCommandHandle[SignalType::WEB_SET_LIGHT4_DATA_RESPONSE] ||
        command == mCommandHandle[SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE] || command == mCommandHandle[SignalType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP])
    {
        std::map<SignalType, String>::iterator it;
        for( it = mCommandHandle.begin(); it != mCommandHandle.end(); it++)
        {
            if (it->second == command)
            {
                mProcessor->handleSignal(it->first);
                break;
            }
        }
    }
    else if (command == mCommandHandle[SignalType::WEB_GET_ALLTIME_DATA_RESPONSE] || command == mCommandHandle[SignalType::WEB_GET_LIGHT1_DATA_RESPONSE] ||
            command == mCommandHandle[SignalType::WEB_GET_LIGHT2_DATA_RESPONSE] || command == mCommandHandle[SignalType::WEB_GET_LIGHT3_DATA_RESPONSE] ||
            command == mCommandHandle[SignalType::WEB_GET_LIGHT4_DATA_RESPONSE] || command == mCommandHandle[SignalType::WEB_GET_STATUS_DATA_RESPONSE])
    {
        std::map<SignalType, String>::iterator it;
        for( it = mCommandHandle.begin(); it != mCommandHandle.end(); it++)
        {
            if (it->second == command)
            {
                int size = 0;
                int *data = parseCommandStringToArray(receiverData, size);
                Package *package = new Package(data, size);
                mProcessor->handleSignal(it->first, package);
                delete package;
                break;
            }
        }
    }
}