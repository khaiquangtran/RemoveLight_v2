#include "./Processor.h"

Processor::Processor()
{
    // mWIFI = std::make_shared<WifiPartner>(this);
    mSERIAL = std::make_shared<SerialPartner>(this);
}

Processor::~Processor()
{
}

void Processor::run()
{
    mSERIAL->listen();
}