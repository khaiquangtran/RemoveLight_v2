#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <memory>
#include "./Network/Network.h"
#include "./Network/WifiPartner.h"
#include "./Network/SerialPartner.h"

class Processor
{
public:
	Processor();
	~Processor();

    void run();
    void init();
    void handleSignal(const SignalType signal, Package *data = nullptr);

private:
    std::shared_ptr<Network> mWIFI;
    std::shared_ptr<Network> mSERIAL;

	int8_t mFlagConnectFirebase;
	int8_t mFlagConnectNTP;
};

#endif // PROCESSOR_H