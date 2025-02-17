#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <memory>
#include "./Hardware/Hardware.h"
#include "./Hardware/WifiPartner.h"
#include "./Hardware/SerialPartner.h"
#include "./Utils/SignalType.h"
#include "./Utils/Package.h"

class Processor
{
public:
	Processor();
	~Processor();

    void run();
    void init();
    void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
    std::shared_ptr<Hardware> mWIFI;
    std::shared_ptr<Hardware> mSERIAL;
};

#endif // PROCESSOR_H