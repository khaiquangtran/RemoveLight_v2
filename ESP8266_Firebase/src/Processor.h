#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <memory>
// #include "./WifiPartner.h"
#include "./SerialPartner.h"
// #include "../RemoteLight/src/Logging.h"

class Processor
{
public:
	Processor();
	~Processor();

    void run();

private:
    // std::shared_ptr<WifiPartner> mWIFI;
    std::shared_ptr<SerialPartner> mSERIAL;
};

#endif // PROCESSOR_H