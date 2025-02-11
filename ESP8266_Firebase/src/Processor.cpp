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
    while (mWIFI->connectWifi() == false)
    {
        // Serial.print(".");
        delay(1000);
    }

}

void Processor::run()
{
    mSERIAL->listen();
}

void Processor::handleSignal(const SignaLType signal, Package *data)
{

}