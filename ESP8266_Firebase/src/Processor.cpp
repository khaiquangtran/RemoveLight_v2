#include "./Processor.h"

Processor::Processor()
{
    mSERIAL = std::make_shared<SerialPartner>(this);
    mWIFI = std::make_shared<WifiPartner>(this);

    mFlagConnectFirebase = 0;
    mFlagConnectNTP = 0;
}

Processor::~Processor()
{
}

void Processor::init()
{
    // while (mWIFI->connectWifi() == false)
    // {
    //     // Serial.print(".");
    //     delay(1000);
    // }
}

void Processor::run()
{
    mSERIAL->listen();
    if (mFlagConnectFirebase == 1 && mFlagConnectNTP == 1)
    {
        mWIFI->handleSignal(SignalType::CHECK_COMMAND_FIREBASE);
    }
}

void Processor::handleSignal(const SignalType signal, Package *data)
{
    // Serial.println(signal);
    switch (signal)
    {
    case SignalType::STATUS_WIFI:
    case SignalType::STATUS_NTP:
    case SignalType::STATUS_FIREBASE:
    case SignalType::REMOTE_LIGHT_GET_TIME_DATE_FROM_NTP:
    case SignalType::WEB_SET_ALLTIME_DATA_RESPONSE:
    case SignalType::WEB_SET_STATUS_LIGHT_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_SET_LIGHT4_DATA_RESPONSE:
        mWIFI->handleSignal(signal);
        break;
    case SignalType::CONNECT_WIFI_FAILED:
    case SignalType::CONNECT_WIFI_SUCCESSFULL:
    case SignalType::WEB_GET_ALLTIME_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_GET_LIGHT4_DATA_REQUEST:
    case SignalType::WEB_GET_STATUS_DATA_REQUEST:
        mSERIAL->handleSignal(signal);
        break;
    case SignalType::CONNECT_FIREBASE_FAILED:
        mFlagConnectFirebase = -1;
        mSERIAL->handleSignal(signal);
        break;
    case SignalType::CONNECT_FIREBASE_SUCCESSFULL:
        mFlagConnectFirebase = 1;
        mSERIAL->handleSignal(signal);
        break;
    case SignalType::CONNECT_NTP_FAILED:
        mFlagConnectNTP = -1;
        mSERIAL->handleSignal(signal);
        break;
    case SignalType::CONNECT_NTP_SUCCESSFULL:
        mFlagConnectNTP = 1;
        mSERIAL->handleSignal(signal);
        break;
    case SignalType::WEB_GET_ALLTIME_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT1_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT2_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT3_DATA_RESPONSE:
    case SignalType::WEB_GET_LIGHT4_DATA_RESPONSE:
    case SignalType::WEB_GET_STATUS_DATA_RESPONSE:
        mWIFI->handleSignal(signal, data);
        break;
    case SignalType::WEB_SET_ALLTIME_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_SET_STATUS_LIGHT4_DATA_REQUEST:
    case SignalType::REMOTE_LIGHT_SEND_TIME_DATE_FROM_NTP:
    case SignalType::WEB_SET_LIGHT1_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT2_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT3_DATA_REQUEST:
    case SignalType::WEB_SET_LIGHT4_DATA_REQUEST:
        mSERIAL->handleSignal(signal, data);
        break;
    case SignalType::CONNECT_RETRY:
        mSERIAL->handleSignal(signal);
        mFlagConnectFirebase = -1;
        mFlagConnectNTP = -1;
        break;
    default:
        break;
    }
}