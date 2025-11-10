#ifndef TASKS_H
#define TASKS_H

#include <mutex>
#include "./../Utils/Logging.h"
#include "./../Utils/Package.h"
#include "./../Utils/SignalType.h"
#include "./../RemoteLight.h"
#include "./../Network/Network.h"
#include "./../Hardware/Hardware.h"

class RemoteLight;
class Network;
class Hardware;

class Tasks
{
public:
    Tasks(std::shared_ptr<RemoteLight> rml, std::shared_ptr<Hardware> lcd, std::shared_ptr<Hardware> rtc, std::shared_ptr<Hardware> ir, std::shared_ptr<Network> net);
    ~Tasks(){};
    Tasks(const Tasks &) = delete;
    Tasks &operator=(const Tasks &) = delete;

    void handleSignal(const SignalType signal, Package *data = nullptr);

private:
    std::shared_ptr<RemoteLight> mRML;
    std::shared_ptr<Hardware> mLCD;
    std::shared_ptr<Hardware> mRTC;
    std::shared_ptr<Hardware> mIR;
    std::shared_ptr<Network> mNET;

    const uint16_t DELAY_1S  	= 1000U;
	const uint16_t DELAY_3S  	= DELAY_1S * 3;
	const uint16_t DELAY_5S  	= DELAY_1S * 5;

    const uint8_t REPEATS_5  	= 5U;
    const uint8_t REPEATS_10  	= 10U;
    const uint8_t REPEATS_30  	= 30U;

    void connectWifiMode();
    void connectWifiTimeout();

    void connectFirebaseMode();
    void connectFirebaseTimeout();

    void connectNTPMode();
    void connectNTPTimeout();

    void displayAllTime();
    void displayAllTimeTimeout();
    void displayTempPressTimeout();

    void displayReadySetupMode();
    void intoSetupMode();
    void displayEndSetupMode();

    void installIRButton();

    enum class CONNECT_STATUS : uint8_t {
        UNKNOWN,
        SUCCESS,
        FAILED,
    };

    enum class MODE_HANDLE : uint8_t {
        NONE = 0,
        INTO_SETUP_MODE,
        INTO_MENU_MODE,
        MENU_MODE,
        INSTALL_IR_BUTTON,
    };

    MODE_HANDLE mModeHandle;
    uint8_t mCounterConnectWifi;
    uint8_t mCounterDisplayAllTime;

    CONNECT_STATUS mFlagConnectFirebase;

    std::map<MODE_HANDLE, String> mPrintListModeHandle;
};

#endif // TASKS_H