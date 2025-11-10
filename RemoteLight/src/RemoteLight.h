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
#include "./Hardware/EEPROMPartner.h"
#include "./Timer/TimerMgr.h"
#include "./Tasks/Tasks.h"
#include "./Network/Network.h"

class Tasks;
class RemoteLight : public std::enable_shared_from_this<RemoteLight>
{
public:
	RemoteLight();
	~RemoteLight();

	void init();
	void run();

	void handleSignal(const SignalType signal, Package *data = nullptr);

	void onTimeout(const SignalType signal = SignalType::NONE);

	std::shared_ptr<TimerManager> mTimerMgr;
	std::shared_ptr<Timer> mTimerConnectWifi;
	std::shared_ptr<Timer> mTimerDisplayAll;
	std::shared_ptr<Timer> mTimerDisplaySetupMode;
	std::shared_ptr<Timer> mTimerCheckConfiguredTimeForLight;

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
		CHECK_CONNECT_FIREBASE,
		CHECK_CONNECT_NTP,
		DISPLAY_TEMP_PRESS,
	};

	std::shared_ptr<Hardware> mSerial;
	std::shared_ptr<Hardware> mRTC;
	std::shared_ptr<Hardware> mIR;
	std::shared_ptr<Hardware> mLCD;
	std::shared_ptr<Hardware> mBTN;
	std::shared_ptr<Hardware> mLIGHT;
	std::shared_ptr<Hardware> mEEPROM;
	std::shared_ptr<Network> mNetwork;
	std::shared_ptr<Tasks> mTasks;

	const uint16_t DELAY_1S  	= 1000U;
	const uint16_t DELAY_3S  	= DELAY_1S * 3;
	const uint16_t DELAY_5S  	= DELAY_1S * 5;
	const uint16_t DELAY_15S  	= DELAY_1S * 15;
	const uint32_t DELAY_1D  	= 86400000U;
	const uint32_t DELAY_3D  	= DELAY_1D * 3;

	bool mCheckConfiguredTimeForLight;

	std::mutex mMutex;
	std::mutex mMutexControlMode;

	int8_t mFlagConnectNTP;
	int8_t mFlagUpdateRTC;

	CONTROL_MODE mControlMode;

	std::map<CONTROL_MODE, SignalType> mControlModeSignalMap;

	SignalType mFlagTimeout;

	bool mFlagIsStoredSsidPassword;

	void handleControlMode();
	void handleTimeout();

	void setCheckConfiguredTimeForLight(const bool state);
	bool getCheckConfiguredTimeForLight();
	void setControlMode(const CONTROL_MODE state);
	CONTROL_MODE getControlMode();
	void setFlagTimeout(SignalType signal);
	SignalType getFlagTimeout();
};

#endif // ! REMOTE_LIGHT_H