#ifndef REMOTE_LIGHT_H
#define REMOTE_LIGHT_H

#include <iostream>
#include <memory>
#include "./Hardware.h"
#include "./RTC.h"
#include "./LCD16x2.h"
#include "./IRRemotePartner.h"
#include "./Button.h"
#include "./Light.h"
#include "./SerialPartner.h"

class RemoteLight
{
public:
	RemoteLight();
	~RemoteLight();

	void init();
	void run();

	void handleSignal(const SignaLType signal, Package *data = nullptr);

private:
	enum class CONTROL_MODE : uint8_t
	{
		NONE = 0U,
		DISPLAY_ALL,
		SETUP_MODE,
		INTO_SETUP_MODE,
		END_SETUP_MODE,
		MENU_MODE,
		INTO_MENU_MODE,
		CHECK_CONNECT_WIFI,
	};

	std::shared_ptr<Hardware> mSERIAL;
	std::shared_ptr<Hardware> mRTC;
	std::shared_ptr<Hardware> mIR;
	std::shared_ptr<Hardware> mLCD;
	std::shared_ptr<Hardware> mBTN;
	std::shared_ptr<Hardware> mLIGHT;

	CONTROL_MODE mControlMode;

	UL mNow;
	UL mElapsed;

	//       name		  millis second		   duration     counter  end
	std::map<String, std::pair<UL, std::pair<int, std::pair<int, int>>>> mTimerList;

	const uint16_t DURATION_TIMER_1S = 1000U;
	const uint16_t DURATION_TIMER_3S = 3000U;
	const uint8_t NUMBER_30S = 30U;
	const uint8_t NUMBER_10S = 10U;

	int8_t mFlagStatusWifi;
	int8_t mFlagStatusFirebase;
	int8_t mFlagStatusNTP;
	uint8_t mCounter;

	void connectWifiMode();
	void displayAllTime();
	void displaySetupMode();
	void displayEndSetupMode();

	enum STATE_CONNECT : uint8_t
	{
		NONE = 0U,
		WIFI,
		FIREBASE,
		NTP,
	};

	STATE_CONNECT mStateConnect;
};

#endif // ! REMOTE_LIGHT_H