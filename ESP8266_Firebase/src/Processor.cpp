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
    if(mFlagConnectFirebase == 1)
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
        mWIFI->handleSignal(SignalType::STATUS_WIFI);
        break;
    case SignalType::CONNECT_WIFI_FAILED:
        mSERIAL->handleSignal(SignalType::CONNECT_WIFI_FAILED);
        break;
    case SignalType::CONNECT_WIFI_SUCCESSFULL:
        mSERIAL->handleSignal(SignalType::CONNECT_WIFI_SUCCESSFULL);
        break;
    case SignalType::STATUS_FIREBASE:
        mWIFI->handleSignal(SignalType::STATUS_FIREBASE);
        break;
    case SignalType::CONNECT_FIREBASE_FAILED:
        mFlagConnectFirebase = 1;
        mSERIAL->handleSignal(SignalType::CONNECT_FIREBASE_FAILED);
        break;
    case SignalType::CONNECT_FIREBASE_SUCCESSFULL:
        mFlagConnectFirebase = 1;
        mSERIAL->handleSignal(SignalType::CONNECT_FIREBASE_SUCCESSFULL);
        break;
    case SignalType::STATUS_NTP:
        mWIFI->handleSignal(SignalType::STATUS_NTP);
        break;
    case SignalType::CONNECT_NTP_FAILED:
        mFlagConnectNTP = -1;
        mSERIAL->handleSignal(SignalType::CONNECT_NTP_FAILED);
        break;
    case SignalType::CONNECT_NTP_SUCCESSFULL:
        mFlagConnectNTP = 1;
        mSERIAL->handleSignal(SignalType::CONNECT_NTP_SUCCESSFULL);
        break;
    case SignalType::REQUEST_ALLTIME_DATA:
        mSERIAL->handleSignal(SignalType::REQUEST_ALLTIME_DATA);
        break;
    default:
        break;
    }
}