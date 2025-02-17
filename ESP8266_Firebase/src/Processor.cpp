#include "./Processor.h"

Processor::Processor()
{
    mSERIAL = std::make_shared<SerialPartner>(this);
    mWIFI = std::make_shared<WifiPartner>(this);
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
}

void Processor::handleSignal(const SignaLType signal, Package *data)
{
    switch (signal)
    {
    case SignaLType::STATUS_WIFI:
        mSERIAL->handleSignal(SignaLType::CONNECT_WIFI_SUCCESSFULL);
        break;
    case SignaLType::STATUS_FIREBASE:
        mSERIAL->handleSignal(SignaLType::CONNECT_FIREBASE_FAILED);
        break;
    case SignaLType::STATUS_NTP:
        mSERIAL->handleSignal(SignaLType::CONNECT_NTP_FAILED);
        break;
    default:
        break;
    }
}