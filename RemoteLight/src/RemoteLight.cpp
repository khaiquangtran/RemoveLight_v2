#include "./RemoteLight.h"

RemoteLight::RemoteLight()
{
	mSerial = std::make_shared<SerialPartner>(this);
	mRTC = std::make_shared<RTC>(this);
	mIR = std::make_shared<IRRemotePartner>(this);
	mLCD = std::make_shared<LCD16x2>(this);
	mBTN = std::make_shared<Button>(this);
	mLIGHT = std::make_shared<Light>(this);

	setControlMode(CONTROL_MODE::NONE);
	setStateConnect(STATE_CONNECT::NOK);

	mCounterConnectWifi = 0;
	mCounterConnectFirebase = 0;
	mCounterConnectNTP = 0;
	mCounterDisplayAllTime = 0;

	mTimerMgr = std::make_shared<TimerManager>(4);
	mTimerConnectWifi = mTimerMgr->createTimer([this]() {
		this->onTimeout(SignaLType::TIMER_CONNECT_WIFI_SIGNAL);}, DELAY_3S);
	mTimerConnectFirebase = mTimerMgr->createTimer([this]() {
		this->onTimeout(SignaLType::TIMER_CONNECT_FIREBASE_SIGNAL);}, DELAY_3S);
	mTimerConnectNTP = mTimerMgr->createTimer([this]() {
		this->onTimeout(SignaLType::TIMER_CONNECT_NTP_SIGNAL);}, DELAY_3S);

	mTimerDisplayAll = mTimerMgr->createTimer([this]() {
		this->onTimeout(SignaLType::TIMER_DISPLAY_ALL_TIME_SIGNAL);}, DELAY_1S);

	LOGI("Initialization RemoteLight!");
}

RemoteLight::~RemoteLight()
{
}

void RemoteLight::init()
{
	setControlMode(CONTROL_MODE::CHECK_CONNECT_WIFI);
	mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_WIFI);
	setStateConnect(STATE_CONNECT::WIFI);
}

void RemoteLight::handleSignal(const SignaLType signal, Package *data)
{
	// LOGD("Handle signal value: %d", signal);
	switch (signal)
	{
	case (SignaLType::PRESS_BTN_1_SIGNAL):
	case (SignaLType::PRESS_BTN_2_SIGNAL):
	case (SignaLType::PRESS_BTN_3_SIGNAL):
	case (SignaLType::IR_BTN_1_SIGNAL):
	case (SignaLType::IR_BTN_2_SIGNAL):
	case (SignaLType::IR_BTN_3_SIGNAL):
	case (SignaLType::IR_BTN_4_SIGNAL):
	{
		mLIGHT->handleSignal(signal);
		break;
	}
	case (SignaLType::LCD_MENU_MODE_BACk):
	case (SignaLType::LCD_MENU_MODE_OK):
	case (SignaLType::LCD_MOVE_LEFT_MENU_MODE):
	case (SignaLType::LCD_MOVE_RIGHT_MENU_MODE):
	case (SignaLType::LCD_DISPLAY_ALL_TIME):
	{
		mLCD->handleSignal(signal, data);
		break;
	}
	case (SignaLType::IR_BTN_APP_SIGNAL):
	{
		if (getControlMode() != CONTROL_MODE::SETUP_MODE &&
			getControlMode() != CONTROL_MODE::MENU_MODE)
		{
			setControlMode(CONTROL_MODE::SETUP_MODE);
			mLCD->handleSignal(signal);
		}
		break;
	}
	case (SignaLType::IR_BTN_UP_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_INCREASE_VALUE);
		}
		else if (getControlMode() == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_INCREASE_VALUE_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_DOWN_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_DECREASE_VALUE);
		}
		else if (getControlMode() == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_DECREASE_VALUE_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_LEFT_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SHIFT_LEFT_VALUE);
		}
		else if (getControlMode() == CONTROL_MODE::MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_LEFT_MENU_MODE);
		}
		else if (getControlMode() == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_LEFT_INTO_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_RIGHT_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SHIFT_RIGHT_VALUE);
		}
		else if (getControlMode() == CONTROL_MODE::MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_RIGHT_MENU_MODE);
		}
		else if (getControlMode() == CONTROL_MODE::INTO_MENU_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_MOVE_RIGHT_INTO_MENU_MODE);
		}
		break;
	}
	case (SignaLType::IR_BTN_OK_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_SETUP_MODE)
		{
			mRTC->handleSignal(SignaLType::RTC_SETUP_MODE_OK);
		}
		else if (getControlMode() == CONTROL_MODE::MENU_MODE)
		{
			setControlMode(CONTROL_MODE::INTO_MENU_MODE);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignaLType::RTC_MENU_MODE_OK);
		}
		break;
	}
	case (SignaLType::IR_BTN_MENU_SIGNAL):
	{
		if (getControlMode() != CONTROL_MODE::SETUP_MODE &&
			getControlMode() != CONTROL_MODE::MENU_MODE)
		{
			setControlMode(CONTROL_MODE::MENU_MODE);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignaLType::LCD_TURN_ON_LIGHT);
			mLCD->handleSignal(SignaLType::IR_BTN_MENU_SIGNAL);
		}
		break;
	}
	case (SignaLType::IR_BTN_BACK_SIGNAL):
	{
		if (getControlMode() == CONTROL_MODE::INTO_MENU_MODE)
		{
			setControlMode(CONTROL_MODE::MENU_MODE);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mRTC->handleSignal(SignaLType::RTC_BACK_MENU_MODE);
		}
		else if (getControlMode() == CONTROL_MODE::MENU_MODE)
		{
			setControlMode(CONTROL_MODE::DISPLAY_ALL);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		}
		break;
	}
	case (SignaLType::REMOTE_LIGHT_END_SETUP_MODE):
	{
		setControlMode(CONTROL_MODE::END_SETUP_MODE);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_END_SETUP_MODE);
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_WIFI_SUCCESS):
	{
		mTimerConnectWifi->stopTimer();
		mCounterConnectWifi = 0;
		mLCD->handleSignal(SignaLType::LCD_CONNECT_WIFI_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_FIREBASE);
		setStateConnect(STATE_CONNECT::FIREBASE);
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_WIFI_FAILED):
	{
		// Do nothing
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_SUCCESS):
	{
		mLCD->handleSignal(SignaLType::LCD_CONNECT_FIREBASE_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_NTP);
		setStateConnect(STATE_CONNECT::NTP);
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_FIREBASE_FAILED):
	{
		// LOGW("Set mFlagStatusFirebase = 0");
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_NTP_SUCCESS):
	{
		mLCD->handleSignal(SignaLType::LCD_CONNECT_NTP_SUCCESS);
		delay(1000);
		mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		setStateConnect(STATE_CONNECT::NOK);
		setControlMode(CONTROL_MODE::DISPLAY_ALL);
		break;
	}
	case (SignaLType::REMOTE_LIGHT_CONNECT_NTP_FAILED):
	{
		// LOGW("Set mFlagStatusNTP = 0");
		break;
	}
	default:
	{
		LOGW("Signal is not supported yet.");
		break;
	}
	}
}

void RemoteLight::run()
{
	mSerial->listenning();
	mIR->listenning();
	mBTN->listenning();

	switch (getControlMode())
	{
	case (CONTROL_MODE::CHECK_CONNECT_WIFI): {
		connectWifiMode();
		break;
	}
	case (CONTROL_MODE::DISPLAY_ALL): {
		displayAllTime();
		break;
	}
	case (CONTROL_MODE::SETUP_MODE): {
		displaySetupMode();
		break;
	}
	case (CONTROL_MODE::END_SETUP_MODE): {
		displayEndSetupMode();
		break;
	}
	default:
		break;
	}
}

void RemoteLight::connectWifiMode()
{
	if(getStateConnect() == STATE_CONNECT::WIFI)
	{
		mSerial->handleSignal(SignaLType::SERIAL_CHECK_STATUS_WIFI);
		setStateConnect(STATE_CONNECT::NOK);
		mTimerConnectWifi->startTimer();
		LOGI("WIFI connection %d times", mCounterConnectWifi);
	}
	else if(getStateConnect() == STATE_CONNECT::FIREBASE)
	{
		mSerial->handleSignal(SignaLType::SERIAL_CHECK_STATUS_FIREBASE);
		setStateConnect(STATE_CONNECT::NOK);
		mTimerConnectFirebase->startTimer();
		LOGI("FIREBASE connection %d times", mCounterConnectFirebase);
	}
	else if(getStateConnect() == STATE_CONNECT::NTP)
	{
		mSerial->handleSignal(SignaLType::SERIAL_CHECK_STATUS_NTP);
		setStateConnect(STATE_CONNECT::NOK);
		mTimerConnectNTP->startTimer();
		LOGI("NTP connection %d times", mCounterConnectNTP);
	}
}

void RemoteLight::displayAllTime()
{
	setControlMode(CONTROL_MODE::NONE);
	mRTC->handleSignal(SignaLType::RTC_DISPLAY_ALL_TIME);
	mTimerDisplayAll->startTimer();
}

void RemoteLight::displaySetupMode()
{
}

void RemoteLight::displayEndSetupMode()
{
}

void RemoteLight::onTimeout(const SignaLType signal)
{
	switch (signal)
	{
	case SignaLType::TIMER_CONNECT_WIFI_SIGNAL:
		if(mCounterConnectWifi < REPEATS_10)
		{
			mLCD->handleSignal(SignaLType::LCD_DISPLAY_CONNECT_WIFI);
			mCounterConnectWifi++;
			setStateConnect(STATE_CONNECT::WIFI);
		}
		else
		{
			mTimerConnectWifi->stopTimer();
			mCounterConnectWifi = 0;
			mLCD->handleSignal(SignaLType::LCD_CONNECT_WIFI_FAILED);
			setControlMode(CONTROL_MODE::DISPLAY_ALL); // Move to display all time mode
			setStateConnect(STATE_CONNECT::NOK);
			LOGW("WIFI connection FAILED!");
			delay(3000);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		}
		break;
	case SignaLType::TIMER_CONNECT_FIREBASE_SIGNAL:
		if(mCounterConnectFirebase < REPEATS_10)
		{
			mLCD->handleSignal(SignaLType::LCD_DISPLAY_CONNECT_WIFI);
			mCounterConnectFirebase++;
			setStateConnect(STATE_CONNECT::FIREBASE);
		}
		else
		{
			mTimerConnectFirebase->stopTimer();
			setStateConnect(STATE_CONNECT::NOK);
			mCounterConnectFirebase = 0;
			mLCD->handleSignal(SignaLType::LCD_CONNECT_FIREBASE_FAILED);
			LOGW("Firebase connection FAILED!");
			delay(3000);
			setStateConnect(STATE_CONNECT::NTP);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
			mLCD->handleSignal(SignaLType::LCD_DISPLAY_START_CONNECT_NTP);
		}
		break;
	case SignaLType::TIMER_CONNECT_NTP_SIGNAL:
		if(mCounterConnectNTP < REPEATS_10)
		{
			mLCD->handleSignal(SignaLType::LCD_DISPLAY_CONNECT_WIFI);
			mCounterConnectNTP++;
			setStateConnect(STATE_CONNECT::NTP);
		}
		else
		{
			mTimerConnectNTP->stopTimer();
			setStateConnect(STATE_CONNECT::NOK);
			mCounterConnectNTP = 0;
			mLCD->handleSignal(SignaLType::LCD_CONNECT_NTP_FAILED);
			LOGW("NTP connection FAILED!");
			delay(3000);
			setControlMode(CONTROL_MODE::DISPLAY_ALL); // Move to display all time mode
			mLCD->handleSignal(SignaLType::LCD_CLEAR_SCREEN);
		}
		break;
	case SignaLType::TIMER_DISPLAY_ALL_TIME_SIGNAL:
		if(mCounterDisplayAllTime < REPEATS_30)
		{
			mCounterDisplayAllTime++;
			setControlMode(CONTROL_MODE::DISPLAY_ALL);
		}
		else
		{
			mCounterDisplayAllTime = 0;
			setControlMode(CONTROL_MODE::NONE);
			mLCD->handleSignal(SignaLType::LCD_CLEAR_TURN_OFF_SCREEN);
			mTimerDisplayAll->stopTimer();
		}
		break;
	default:
		break;
	}
}

void RemoteLight::setStateConnect(const STATE_CONNECT state)
{
	std::unique_lock<std::mutex> lock(mMutex);
	mStateConnect = state;
}

RemoteLight::STATE_CONNECT RemoteLight::getStateConnect()
{
	std::unique_lock<std::mutex> lock(mMutex);
	return mStateConnect;
}

void RemoteLight::setControlMode(const CONTROL_MODE state)
{
	std::unique_lock<std::mutex> lock(mMutex2);
	mControlMode = state;
}

RemoteLight::CONTROL_MODE RemoteLight::getControlMode()
{
	std::unique_lock<std::mutex> lock(mMutex2);
	return mControlMode;
}