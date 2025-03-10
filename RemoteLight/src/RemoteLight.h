#ifndef REMOTE_LIGHT_H
#define REMOTE_LIGHT_H

#include <iostream>
#include <memory>
#include <mutex>
#include "./Hardware/Hardware.h"
#include "./Hardware/RTC.h"
#include "./Hardware/LCD16x2.h"
#include "./Hardware/IRRemotePartner.h"
#include "./Hardware/Button.h"
#include "./Hardware/Light.h"
#include "./Hardware/SerialPartner.h"
#include "./Timer/TimerMgr.h"
class RemoteLight
{
public:
	RemoteLight();
	~RemoteLight();

	void init();
	void run();

	void handleSignal(const SignaLType signal, Package *data = nullptr);

	void onTimeout(const SignaLType signal = SignaLType::NONE);

private:
	enum class CONTROL_MODE : uint8_t
	{
		NONE = 0U,
		DISPLAY_ALL,
		READY_SETUP_MODE,
		START_SETUP_MODE,
		INTO_SETUP_MODE,
		END_SETUP_MODE,
		MENU_MODE,
		INTO_MENU_MODE,
		CHECK_CONNECT_WIFI,
	};

	enum STATE_CONNECT : uint8_t
	{
		NOK = 0U,
		WIFI,
		FIREBASE,
		NTP,
	};

	std::shared_ptr<Hardware> mSerial;
	std::shared_ptr<Hardware> mRTC;
	std::shared_ptr<Hardware> mIR;
	std::shared_ptr<Hardware> mLCD;
	std::shared_ptr<Hardware> mBTN;
	std::shared_ptr<Hardware> mLIGHT;
	std::shared_ptr<TimerManager> mTimerMgr;

	std::shared_ptr<Timer> mTimerConnectWifi;
	std::shared_ptr<Timer> mTimerDisplayAll;
	std::shared_ptr<Timer> mTimerDisplaySetupMode;
	std::shared_ptr<Timer> mTimerCheckConfiguredTimeForLight;

	const uint16_t DELAY_1S  	= 1000U;
	const uint16_t DELAY_3S  	= 3000U;
	const uint16_t DELAY_5S  	= 5000U;
	const uint32_t DELAY_1D  	= 86400000U;
	const uint32_t DELAY_3D  	= 86400000U*3;
	const uint8_t REPEATS_30  	= 30U;
	const uint8_t REPEATS_10  	= 10U;

	uint8_t mCounterConnectWifi;
	uint8_t mCounterDisplayAllTime;
	bool mCheckConfiguredTimeForLight;

	std::mutex mMutex;
	std::mutex mMutex2;
	std::mutex mMutex3;

	int8_t mFlagConnectNTP;
	int8_t mFlagUpdateRTC;

	STATE_CONNECT mStateConnect;
	CONTROL_MODE mControlMode;

	void connectWifiMode();
	void displayAllTime();
	void displayReadySetupMode();
	void intoSetupMode();
	void displayEndSetupMode();

	void setControlMode(const CONTROL_MODE state);
	CONTROL_MODE getControlMode();

	void setStateConnect(const STATE_CONNECT state);
	STATE_CONNECT getStateConnect();

	void setCheckConfiguredTimeForLight(const bool state);
	bool getCheckConfiguredTimeForLight();
};

#endif // ! REMOTE_LIGHT_H